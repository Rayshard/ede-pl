#pragma once
#include <regex>
#include <variant>
#include <map>
#include <optional>
#include <any>
#include <fstream>
#include <unordered_map>


namespace parser
{
    struct Position
    {
        size_t line, column;

        std::string ToString() const { return "(" + std::to_string(line) + ", " + std::to_string(column) + ")"; }
    };

    std::ostream& operator<<(std::ostream& _lhs, const Position& _rhs) {
        return _lhs << _rhs.ToString();
    }

    class Regex : public std::regex
    {
        std::string string;
    public:
        Regex() : std::regex(), string() { }
        Regex(const std::string& _string) : std::regex(_string), string(_string) { }

        const std::string& GetString() { return string; }
    };

    template<typename T>
    class Stream
    {
    protected:
        std::vector<T> data;

    public:
        size_t offset;

        Stream(std::vector<T>&& _data) : data(std::move(_data)), offset(0)
        {
            assert(!data.empty() && "Data must be non-empty!");
        }

        virtual T Peek() { return IsEOS() ? data.back() : data[offset]; }
        
        void Ignore(size_t _amt)
        {
            for(size_t i = 0; i < _amt; i++)
                Get();
        }

        T Get()
        {
            auto peek = Peek();
            offset++;
            return peek;
        }

        typename std::vector<T>::iterator Begin() { return data.begin(); }
        typename std::vector<T>::iterator End() { return data.end(); }
        typename std::vector<T>::iterator Current() { return data.begin() + offset; }

        typename std::vector<T>::const_iterator CBegin() const { return data.cbegin(); }
        typename std::vector<T>::const_iterator CEnd() const { return data.cend(); }
        typename std::vector<T>::const_iterator CCurrent() const { return data.cbegin() + offset; }

        bool IsEOS() { return offset >= data.size(); }
    };

    class StringStream : public Stream<char>
    {
        std::vector<size_t> lineStarts;

    public:
        StringStream(std::string&& _data) : Stream(std::vector<char>(_data.begin(), _data.end()))
        {
            //Get line starts
            lineStarts.push_back(0);

            for (size_t position = 0; position < _data.size(); position++)
            {
                if (_data[position] == '\n')
                    lineStarts.push_back(position + 1);
            }
        }

        StringStream(std::istream& _stream) : StringStream(std::string((std::istreambuf_iterator<char>(_stream)), std::istreambuf_iterator<char>())) { }

        std::string GetDataAsString(size_t _start, size_t _length) const
        {
            if (_start + _length >= data.size())
                throw std::runtime_error("Parameters out of range of data!");

            auto start = data.begin() + _start;
            return std::string(start, start + _length);
        }

        Position GetPosition(size_t _offset) const
        {
            if (_offset > data.size())
                throw std::runtime_error("Offset is out of range of data!");

            size_t line = 0, closestLineStart = 0;
            for (size_t lineStart : lineStarts)
            {
                if (lineStart > _offset)
                    break;

                closestLineStart = lineStart;
                line++;
            }

            return Position{ line, _offset - closestLineStart + 1 };
        }

        Position GetPosition() const { return GetPosition(offset); }

        void SetPosition(Position _pos)
        {
            if (_pos.line > lineStarts.size() || _pos.line == 0 || _pos.column == 0)
                throw std::runtime_error("Invaild position: " + _pos.ToString());

            size_t lineStart = lineStarts[_pos.line - 1];
            size_t lineWidth = (_pos.line == lineStarts.size() ? data.size() : lineStarts[_pos.line]) - lineStart;

            if (_pos.column - 1 > lineWidth)
                throw std::runtime_error("Invaild position: " + _pos.ToString());

            offset = lineStart + _pos.column - 1;
        }
    };

    template<typename Unit, class TStream, typename PatternTemplate, typename Match>
        requires(std::is_base_of<Stream<Unit>, TStream>::value)
    class PatternMatcher
    {
    public:
        typedef size_t PatternID;
        inline static constexpr PatternID EOS_PATTERN_ID() { return 0; }
        inline static constexpr PatternID UNKNOWN_PATTERN_ID() { return 1; }

        typedef std::monostate NoAction;
        typedef void(*Procedure)(TStream&, const Match&);
        typedef std::any(*Function)(TStream&, const Match&);
        typedef std::variant<Procedure, Function, NoAction> Action;

        struct Result
        {
            PatternID patternID;
            std::any value;
            Match match;

            bool IsEOS() { return patternID == EOS_PATTERN_ID(); }
            bool IsUnknown() { return patternID == UNKNOWN_PATTERN_ID(); }
        };

    private:
        struct Pattern
        {
            PatternID id;
            PatternTemplate pTemplate;
            Action action;
        };

        std::vector<Pattern> patterns;
        Pattern eos, unknown;

    public:
        PatternMatcher(Action _onEOS, Action _onUnknown) : patterns()
        {
            eos = { .id = EOS_PATTERN_ID(), .pTemplate = PatternTemplate(), .action = _onEOS };
            unknown = { .id = UNKNOWN_PATTERN_ID(), .pTemplate = PatternTemplate(), .action = _onUnknown };
        }

        PatternID AddPattern(PatternTemplate _template, Action _action = NoAction())
        {
            patterns.push_back(Pattern{ .id = patterns.size() + 2, .pTemplate = _template, .action = _action });
            return patterns.back().id;
        }

        Result GetMatch(TStream& _stream)
        {
            Pattern* matchingPattern = nullptr;
            size_t streamStart = _stream.offset;
            Match match;

            if (_stream.IsEOS())
            {
                matchingPattern = &eos;
                match = MatchEOS(_stream);
            }
            else
            {
                size_t greatestPatternMatchLength = 0;

                for (auto& pattern : patterns)
                {
                    auto patternMatch = MatchPatternTemplate(_stream, pattern.pTemplate); //Call to this function should update the stream position past the match
                    size_t patternMatchLength = _stream.offset - streamStart;

                    _stream.offset = streamStart; //Reset the stream's position to before match so that possible future calls to the matcher start at the same offset

                    if (!patternMatch.has_value())
                        continue;

                    //If this is the first match or this match has a strictly greater length than
                    //any previous match, set it as the match 
                    if (!matchingPattern || patternMatchLength > greatestPatternMatchLength)
                    {
                        matchingPattern = &pattern;
                        match = patternMatch.value();
                        greatestPatternMatchLength = patternMatchLength;
                    }
                }

                if (!matchingPattern)
                {
                    matchingPattern = &unknown;
                    match = MatchUnknown(_stream);
                }
                else { _stream.Ignore(greatestPatternMatchLength); }
            }

            Result result = {
                .patternID = matchingPattern->id,
                .value = std::any(),
                .match = match
            };

            if (auto action = std::get_if<Function>(&matchingPattern->action)) { result.value = (*action)(_stream, match); }
            else if (auto action = std::get_if<Procedure>(&matchingPattern->action)) { (*action)(_stream, match); }

            return result;
        }

        const std::vector<Pattern>& GetPatterns() const { return patterns; }

    private:
        virtual Match MatchEOS(TStream& _stream) = 0;
        virtual Match MatchUnknown(TStream& _stream) = 0;
        virtual std::optional<Match> MatchPatternTemplate(TStream&, const PatternTemplate&) = 0;
    };

    struct LexerMatch
    {
        Position position;
        std::string value;
    };

    class Lexer : public PatternMatcher<char, StringStream, Regex, LexerMatch>
    {
    public:
        Lexer(Action _onEOS, Action _onUnknown) : PatternMatcher<char, StringStream, Regex, LexerMatch>(_onEOS, _onUnknown) { }

    private:
        LexerMatch MatchEOS(StringStream& _stream) override { return LexerMatch{ .position = _stream.GetPosition(), .value = std::string(1, (char)EOF) }; }
        LexerMatch MatchUnknown(StringStream& _stream) override { return LexerMatch{ .position = _stream.GetPosition(), .value = std::string(1, _stream.Get()) }; }

        std::optional<LexerMatch> MatchPatternTemplate(StringStream& _stream, const Regex& _regex) override
        {
            Position position = _stream.GetPosition();
            const char* current = &*_stream.CCurrent();
            const char* end = &*_stream.CEnd();
            std::cmatch regexMatch;
            std::regex_search(current, end, regexMatch, _regex, std::regex_constants::match_continuous);

            if (regexMatch.size() == 0)
                return std::optional<LexerMatch>();

            _stream.Ignore(regexMatch.length()); //Advance stream past match
            return LexerMatch{ .position = position, .value = regexMatch.str() };
        }
    };

    class TokenStream : public Stream<Lexer::Result>
    {
        Lexer* lexer;
        StringStream* ss;
    public:
        TokenStream(Lexer* _lexer, StringStream* _ss) : Stream({ _lexer->GetMatch(*_ss) }), lexer(_lexer), ss(_ss) { }

        Lexer::Result Peek() override
        {
            assert(!data.empty() && "A token stream should be non-empty, containing at least the EOS token!");

            while (!data.back().IsEOS() && offset >= data.size() - 1)
                data.push_back(lexer->GetMatch(*ss));

            return Stream<Lexer::Result>::Peek();
        }

        Position GetPosition() { return Peek().match.position; }
    };

    class Parser
    {
    public:
        typedef std::string SymbolID;
        static constexpr const char* SymbolID_InvalidTerminal = "<INVALID>";
        static constexpr const char* SymbolID_AnyTerminal = "<TERMINAL>";
        static constexpr const char* SymbolID_AnySymbol = "<SYMBOL>";

        struct Result;

        typedef Lexer::PatternID Terminal;
        typedef std::vector<SymbolID> Rule;
        typedef LexerMatch TerminalMatch;
        typedef std::vector<Parser::Result> NTMatch;

        class Result
        {
        public:
            typedef std::variant<TerminalMatch, NTMatch> Match;

        private:
            SymbolID id;
            Position position;
            std::any value;
            Match match;

        public:
            Result(SymbolID _id, Position _pos, std::any&& _val, Match&& _match)
                : id(_id), position(_pos), value(std::move(_val)), match(std::move(_match)) { }

            const SymbolID& GetID() const { return id; }
            const Position& GetPosition() const { return position; }
            const std::any& GetValue() const { return value; }
            const TerminalMatch& GetMatchAsTerminal() const { return std::get<TerminalMatch>(match); }
            const NTMatch& GetMatchAsNonTerminal() const { return std::get<NTMatch>(match); }

            template<typename T>
            bool IsMatch() const { return std::get_if<T>(&match); }

            template<typename T>
            const T& GetValue() const { return std::any_cast<const T&>(value); }
        };

        class NonTerminal : public PatternMatcher<Lexer::Result, TokenStream, Rule, NTMatch>
        {
        public:
            Parser* parser;

            NonTerminal(Parser* _parser) : PatternMatcher<Lexer::Result, TokenStream, Rule, NTMatch>(NoAction(), NoAction()), parser(_parser) { }

        private:
            NTMatch MatchEOS(TokenStream& _stream) override { return { }; }

            NTMatch MatchUnknown(TokenStream& _stream) override
            {
                auto token = _stream.Get();
                return { Parser::Result(SymbolID_InvalidTerminal, token.match.position, std::move(token.value), Parser::Result::Match()) };
            }

            std::optional<NTMatch> MatchPatternTemplate(TokenStream& _stream, const Rule& _rule) override
            {
                NTMatch match;

                for (auto& component : _rule)
                {
                    try
                    {
                        auto arg = parser->ParseSymbol(_stream, component);

                        if (component == SymbolID_AnyTerminal)
                        {
                            auto innerResult = arg.GetMatchAsNonTerminal()[0];

                            assert(innerResult.IsMatch<TerminalMatch>() && "Each rule inside of <TERMINAL> should match to a terminal!");
                            arg = innerResult;
                        }
                        else if (component == SymbolID_AnySymbol) { arg = arg.GetMatchAsNonTerminal()[0]; }

                        match.push_back(arg);
                    }
                    catch (const std::runtime_error& e) { return std::optional<NTMatch>(); }
                }

                return match;
            }
        };

    private:
        typedef std::variant<Terminal, NonTerminal> Symbol;
        std::unordered_map<SymbolID, Symbol> symbols;
        std::unordered_map<Terminal, SymbolID> terminals;

        bool validated;
    public:
        Lexer lexer;

        Parser(Lexer::Action _onEOS, Lexer::Action _onUnknown) : lexer(_onEOS, _onUnknown), validated(true)
        {
            symbols.emplace(SymbolID_AnyTerminal, NonTerminal(this));
            symbols.emplace(SymbolID_AnySymbol, NonTerminal(this));

            AddTerminal(SymbolID_InvalidTerminal, Lexer::UNKNOWN_PATTERN_ID());
        }
    private:
        void AddSymbol(SymbolID _id, Symbol _symbol)
        {
            if (_id.empty()) { throw std::runtime_error("Symbol id must be non-empty!"); }
            else if (!symbols.emplace(_id, _symbol).second) { throw std::runtime_error("Symbol ID '" + _id + "' is already in use."); }

            if (auto terminal = std::get_if<Terminal>(&_symbol))
            {
                terminals.emplace(*terminal, _id);
                AddRule("<TERMINAL>", { _id });
            }

            AddRule("<SYMBOL>", { _id });
        }

        Result ParseTerminal(TokenStream& _stream, SymbolID _terminalSymbolID)
        {
            size_t parseStart = _stream.offset;
            Lexer::Result token;

            while (true)
            {
                token = _stream.Get();

                auto terminalSearch = terminals.find(token.patternID);
                if (terminalSearch == terminals.end())
                {
                    if (token.IsEOS())
                    {
                        _stream.offset = parseStart;
                        throw std::runtime_error("Expected " + _terminalSymbolID + " but lexer reached end of file.");
                    }
                    else { continue; }
                }

                if (terminalSearch->second == _terminalSymbolID)
                    return Result(terminalSearch->second, token.match.position, std::move(token.value), token.match);

                _stream.offset = parseStart;
                throw std::runtime_error("Expected " + _terminalSymbolID + " but found '" + terminalSearch->second + "'");
            }
        }

        Result ParseNonTerminal(TokenStream& _stream, SymbolID _ntSymbolID)
        {
            NonTerminal& nt = std::get<NonTerminal>(symbols.at(_ntSymbolID));
            Position streamStart = _stream.GetPosition();
            auto matchedRule = nt.GetMatch(_stream);

            switch (matchedRule.patternID)
            {
            case NonTerminal::EOS_PATTERN_ID(): throw std::runtime_error("Unable to parse '" + _ntSymbolID + "'. Encountered end of stream!");
            case NonTerminal::UNKNOWN_PATTERN_ID(): throw std::runtime_error("Unable to parse '" + _ntSymbolID + "'. No rule matches the stream's tokens.");
            default: return Result(_ntSymbolID, matchedRule.match.empty() ? streamStart : matchedRule.match[0].GetPosition(), std::move(matchedRule.value), matchedRule.match);
            }
        }

        Result ParseSymbol(TokenStream& _stream, SymbolID _symbolID)
        {
            auto symbolSearch = symbols.find(_symbolID);
            if (symbolSearch == symbols.end())
                throw std::runtime_error("Symbol with id '" + _symbolID + "' does not exist!");

            return std::get_if<Terminal>(&symbolSearch->second) ? ParseTerminal(_stream, _symbolID) : ParseNonTerminal(_stream, _symbolID);
        }

    public:
        void AddTerminal(SymbolID _id, Lexer::PatternID _patternID) { AddSymbol(_id, _patternID); }
        void AddTerminal(SymbolID _id, Regex _regex, Lexer::Action _action = NonTerminal::NoAction()) { AddSymbol(_id, lexer.AddPattern(_regex, _action)); }

        void AddRule(SymbolID _ntID, std::initializer_list<SymbolID> _components, NonTerminal::Action _action = NonTerminal::NoAction())
        {
            auto symbolSearch = symbols.find(_ntID);
            if (symbolSearch == symbols.end()) { AddSymbol(_ntID, NonTerminal(this)); }
            else if (std::get_if<Terminal>(&symbolSearch->second)) { throw std::runtime_error("'" + _ntID + "' refers to a terminal. Rules can only be added to non-terminals!"); }

            std::get<NonTerminal>(symbols.at(_ntID)).AddPattern(_components, _action);
            validated = false;
        }

        void Validate()
        {
            for (auto& [symbolID, symbol] : symbols)
            {
                if (auto nt = std::get_if<NonTerminal>(&symbol))
                {
                    for (auto& rule : nt->GetPatterns())
                    {
                        for (auto& component : rule.pTemplate)
                        {
                            auto symbolSearch = symbols.find(component);
                            if (symbolSearch == symbols.end())
                                throw std::runtime_error("Nonterminal '" + symbolID + "' references a symbol that does not exist: '" + component + "'");
                        }
                    }
                }
            }

            validated = true;
        }

        Result Parse(StringStream& _stream, SymbolID _symbolID)
        {
            if (!validated)
                Validate();

            size_t streamStart = _stream.offset;

            try
            {
                TokenStream tStream(&lexer, &_stream);
                return ParseSymbol(tStream, _symbolID);
            }
            catch (const std::runtime_error& e)
            {
                _stream.offset = streamStart;
                throw e;
            }
        }
    };
}