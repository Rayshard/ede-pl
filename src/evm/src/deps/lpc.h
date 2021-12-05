#pragma once
#include <string>
#include <regex>
#include <vector>
#include <fstream>
#include <variant>
#include <optional>
#include <set>
#include <any>
#include <functional>
#include <sstream>
#include <tuple>
#include <unordered_map>

namespace lpc
{
    //Represents a line and column pair
    struct Position
    {
        size_t line, column;

        std::string ToString() const;
    };

    std::ostream& operator<<(std::ostream& _lhs, const Position& _rhs);

    class Regex : public std::regex
    {
        std::string string;
    public:
        Regex();
        Regex(const std::string& _string);

        const std::string& GetString() const;
    };

    /**
     * Converts the remainder of an std::istream into a string without modifying the stream
     *
     * @param _stream std::istream the stream to convert
     * @return std::string representing the remainder of the stream
     */
    std::string IStreamToString(std::istream& _stream);

    template<typename T>
    class Parser;

    class StringStream;

    class Lexer
    {
    public:
        typedef std::string PatternID;
        inline static PatternID EOS_PATTERN_ID() { return "<EOS>"; }
        inline static PatternID UNKNOWN_PATTERN_ID() { return "<UNKNOWN>"; }

        struct Token
        {
            PatternID patternID;
            Position position;
            std::string value;

            bool IsEOS() const;
            bool IsUnknown() const;
        };

        typedef std::monostate NoAction;
        typedef std::function<void(StringStream&, const Token&)> Procedure;
        typedef std::function<std::string(StringStream&, const Token&)> Function;
        typedef std::variant<Procedure, Function, NoAction> Action;

        struct Pattern
        {
            PatternID id;
            Regex regex;
            Action action;

            Parser<std::string> AsTerminal(std::optional<std::string> _value = std::nullopt) const;
        };

    private:
        std::vector<Pattern> patterns;
        std::unordered_map<PatternID, Pattern*> patternsMap;
        Pattern patternEOS, patternUnknown;

    public:
        Lexer(Action _onEOS = std::monostate(), Action _onUnknown = std::monostate());

        const Pattern& AddPattern(PatternID _id, Regex _regex, Action _action = NoAction());
        const Pattern& AddPattern(Regex _regex, Action _action = NoAction());

        Token Lex(StringStream& _stream) const;

        const Pattern& GetPattern(const PatternID& _id) const;
        bool HasPattern(const PatternID& _id) const;
    };

    class StringStream
    {
        size_t offset;
        std::string data;
        std::vector<size_t> lineStarts;

        const Lexer lexer;
        std::unordered_map<size_t, std::pair<Lexer::Token, size_t>> tokens;
        std::set<Lexer::PatternID> ignores;

    public:
        StringStream(const std::string& _data, const Lexer& _lexer, const std::set<Lexer::PatternID>& _ignores = {});
        StringStream(const std::string& _data);

        char GetChar();
        char PeekChar();
        void IgnoreChars(size_t _amt);

        const Lexer::Token& GetToken();
        const Lexer::Token& PeekToken();

        size_t GetOffset() const;
        size_t GetOffset(const Position& _pos) const;
        Position GetPosition(size_t _offset) const;
        Position GetPosition() const;

        void SetOffset(size_t _offset);
        void SetPosition(Position _pos);

        std::string GetData(size_t _start) const;
        std::string GetData(size_t _start, size_t _length) const;

        std::string::iterator Begin();
        std::string::iterator End();
        std::string::iterator Current();

        std::string::const_iterator CBegin() const;
        std::string::const_iterator CEnd() const;
        std::string::const_iterator CCurrent() const;

        bool IsEOS() const;
    };

    class ParseError : public std::runtime_error
    {
        Position position;
        std::string message, details;

    public:
        ParseError(Position _pos, const std::string& _msg, const std::string& _details = "");

        ParseError(const ParseError& _e1, const ParseError& _e2);

        const Position& GetPosition() const;
        const std::string& GetMessage() const;
        const std::string& GetDetails() const;

        static ParseError Expectation(const std::string& _expected, const std::string& _found, const Position& _pos);
    };

    template<typename T>
    struct ParserOperation
    {
        std::string name;

        ParserOperation(const std::string& _name) : name(_name) { }

        virtual Parser<T> GetParser() const = 0;
    };

    template<typename T>
    struct ParseResult
    {
        Position position;
        T value;

        ParseResult(Position _pos, T _value) : position(_pos), value(_value) {}
        ParseResult() : ParseResult(Position(), T()) {}
    };

    template<typename T>
    class Parser
    {
    public:
        typedef ParseResult<T> Result;
        typedef std::function<Result(const Position& _pos, StringStream& _stream)> Function;

    private:
        std::string name;
        Function function;

    public:
        Parser(const std::string& _name, Function _func) : name(_name), function(_func) { }

        Parser() : Parser("", [](const Position& _pos, StringStream& _stream) { return Result(_pos, T()); }) { }

        Parser(const std::string& _name, Parser<T> _parser) : Parser(_name, [=](const Position& _pos, StringStream& _stream) { return _parser.Parse(_stream); }) { }

        Parser(const ParserOperation<T>& _op) : Parser(_op.GetParser()) { }

        Parser(const std::string& _name, const ParserOperation<T>& _op) : Parser(_name, _op.GetParser()) { }

        Result Parse(StringStream& _stream) const
        {
            auto streamStart = _stream.GetOffset();

            try { return function(_stream.GetPosition(), _stream); }
            catch (const ParseError& e)
            {
                _stream.SetOffset(streamStart);
                throw ParseError(e, ParseError(_stream.GetPosition(), "Unable to parse " + name));
            }
        }

        Result Parse(const std::string& _input, const Lexer& _lexer, const std::set<Lexer::PatternID>& _ignores = {}) const
        {
            StringStream stream(_input, _lexer, _ignores);
            return Parse(stream);
        };

        Result Parse(const std::string& _input) const { return Parse(_input, Lexer()); };

        template<typename M>
        using Mapper = std::function<M(const Result&)>;

        template<typename M>
        Parser<M> Map(const std::string& _name, Mapper<M> _func) const
        {
            return Parser<M>(_name, [=, function = function](const Position& _pos, StringStream& _stream)
                {
                    auto result = function(_pos, _stream);
                    return ParseResult<M>(result.position, _func(result));
                });
        }

        template<typename M>
        Parser<M> Map(Mapper<M> _func) const { return Map(name, _func); }

        Parser<T> Satisfy(std::function<bool(const Result&)> _predicate, std::function<std::string(const Result&)> _onFail) const
        {
            return Parser<T>(name, [=, function = function](const Position& _pos, StringStream& _stream)
                {
                    auto result = function(_pos, _stream);

                    if (_predicate(result)) { return result; }
                    else { throw ParseError(result.position, "Failed to satisfy predicate: " + _onFail(result)); }
                });
        }

        Parser<T> Satisfy(const T& _value, std::function<std::string(const T&)> _onFail = nullptr) const
        {
            return Parser<T>(name, [=, function = function](const Position& _pos, StringStream& _stream)
                {
                    auto result = function(_pos, _stream);

                    if (result.value == _value) { return result; }
                    else { throw ParseError(result.position, _onFail ? _onFail(result.value) : (name + " was not satisfied")); }
                });
        }

        template<typename C>
        Parser<C> Chain(std::function<Parser<C>(Result&)> _chainer) const
        {
            return Parser<C>(name, [=, function = function](const Position& _pos, StringStream& _stream)
                {
                    auto result = function(_pos, _stream);
                    return _chainer(result).function(_pos, _stream);
                });
        }

        bool ParsesTo(const T& _value, const std::string& _input, const Lexer& _lexer, const std::set<Lexer::PatternID>& _ignores = {}) { return Parse(_input, _lexer, _ignores).value == _value; }

        bool ParsesTo(const T& _value, const std::string& _input) { return ParsesTo(_input, Lexer()); }

        bool ErrorsOn(const std::string& _input, const Lexer& _lexer, const std::set<Lexer::PatternID>& _ignores = {})
        {
            try
            {
                Parse(_input, _lexer, _ignores);
                return false;
            }
            catch (const ParseError& e) { return true; }
        }

        bool ErrorsOn(const std::string& _input) { return ErrorsOn(_input, Lexer()); }

        const std::string& GetName() const { return name; }
    };

    template<typename T>
    class LPC
    {
    public:
        typedef std::tuple<Lexer, Parser<T>, std::set<Lexer::PatternID>> Tuple;

    private:
        Lexer lexer;
        Parser<T> parser;
        std::set<Lexer::PatternID> ignores;

    public:
        LPC(const Lexer& _lexer, const Parser<T>& _parser, const std::set<Lexer::PatternID>& _ignores = {}) : lexer(_lexer), parser(_parser), ignores(_ignores) { }
        LPC(const Tuple& _lpc) : LPC(std::get<0>(_lpc), std::get<1>(_lpc), std::get<2>(_lpc)) { }

        ParseResult<T> Parse(const std::string& _input) { return parser.Parse(_input, lexer, ignores); }
    };

#pragma region Try
    template<typename Success, typename Error>
    class TryValue
    {
        std::variant<Success, std::pair<Error, ParseError>> result;

    public:
        TryValue(const std::variant<Success, std::pair<Error, ParseError>>& _result = Success()) : result(_result) { }

        bool IsSuccess() const { return result.index() == 0; }
        bool IsError() const { return result.index() == 1; }

        const Success& GetSuccess() const { return std::get<0>(result); }
        const Error& GetError() const { return std::get<0>(std::get<1>(result)); }
        const ParseError& GetParseError() const { return std::get<1>(std::get<1>(result)); }

        static TryValue CreateSuccess(const Success& _success) { return TryValue(std::variant<Success, std::pair<Error, ParseError>>(std::in_place_index<0>, _success)); }
        static TryValue CreateError(const Error& _error, const ParseError& _parseError) { return TryValue(std::variant<Success, std::pair<Error, ParseError>>(std::in_place_index<1>, std::make_pair(_error, _parseError))); }
    };

    template<typename Success, typename Error>
    using TryParser = Parser<TryValue<Success, Error>>;

    template<typename Success, typename Error>
    using TryResult = ParseResult<TryValue<Success, Error>>;

    template<typename Success, typename Error>
    TryParser<Success, Error> Try(const Parser<Success>& _parser, const Error& _error)
    {
        return TryParser<Success, Error>("TRY(" + _parser.GetName() + ")", [=](const Position& _pos, StringStream& _stream)
            {
                try
                {
                    auto result = _parser.Parse(_stream);
                    return TryResult<Success, Error>(result.position, TryValue<Success, Error>::CreateSuccess(result.value));
                }
                catch (const ParseError& e)
                {
                    return TryResult<Success, Error>(_pos, TryValue<Success, Error>::CreateError(_error, e));
                }
            });
    }

    template<typename T>
    TryParser<T, std::monostate> Try(const Parser<T>& _parser) { return Try<T, std::monostate>(_parser, std::monostate()); }
#pragma endregion

#pragma region Optional
    template<typename T>
    using OptionalValue = std::optional<T>;

    template<typename T>
    using OptionalParser = Parser<OptionalValue<T>>;

    template<typename T>
    using OptionalResult = ParseResult<OptionalValue<T>>;

    template<typename T>
    OptionalParser<T> Optional(const Parser<T>& _parser)
    {
        return OptionalParser<T>(_parser.GetName() + "?", [=](const Position& _pos, StringStream& _stream)
            {
                std::optional<T> value;

                try
                {
                    auto result = _parser.Parse(_stream);
                    return OptionalResult<T>(result.position, result.value);
                }
                catch (const ParseError& e) { return OptionalResult<T>(_pos, std::nullopt); }
            });
    }
#pragma endregion

#pragma region Quantified
    template<typename T>
    using QuantifiedValue = std::vector<ParseResult<T>>;

    template<typename T>
    using QuantifiedParser = Parser<QuantifiedValue<T>>;

    template<typename T>
    using QuantifiedResult = ParseResult<QuantifiedValue<T>>;

    template<typename T>
    QuantifiedParser<T> Quantified(const std::string& _name, const Parser<T>& _parser, size_t _min, size_t _max)
    {
        assert(_max >= _min && "_max must be at least _min");

        return QuantifiedParser<T>(_name, [=](const Position& _pos, StringStream& _stream)
            {
                QuantifiedValue<T> results;

                if (_max != 0)
                {
                    while (results.size() <= _max)
                    {
                        try { results.push_back(_parser.Parse(_stream)); }
                        catch (const ParseError& e)
                        {
                            if (results.size() >= _min)
                                break;

                            auto error = ParseError::Expectation("at least " + std::to_string(_min) + " '" + _parser.GetName() + "'", "only " + std::to_string(results.size()), _stream.GetPosition());
                            throw ParseError(e, error);
                        }
                    }
                }

                Position position = results.size() == 0 ? _pos : results[0].position; //this is separated from the return statement because of the unknown order of argument evaluation 
                return QuantifiedResult<T>(position, std::move(results));
            });
    }

    template<typename T>
    QuantifiedParser<T> ManyOrOne(const std::string& _name, const Parser<T>& _parser) { return Quantified<T>(_name, _parser, 1, -1); }

    template<typename T>
    QuantifiedParser<T> ZeroOrOne(const std::string& _name, const Parser<T>& _parser) { return Quantified<T>(_name, _parser, 0, 1); }

    template<typename T>
    QuantifiedParser<T> ZeroOrMore(const std::string& _name, const Parser<T>& _parser) { return Quantified<T>(_name, _parser, 0, -1); }

    template<typename T>
    QuantifiedParser<T> Exactly(const std::string& _name, const Parser<T>& _parser, size_t _n) { return Quantified<T>(_name, _parser, _n, _n); }
#pragma endregion

#pragma region List
    template<typename... Ts>
    struct List : public ParserOperation<std::tuple<ParseResult<Ts>...>>
    {
        using Parsers = std::tuple<Parser<Ts>...>;
        using ParserT = Parser<std::tuple<ParseResult<Ts>...>>;
        using ParserTValue = std::tuple<ParseResult<Ts>...>;
        using ParserTResult = ParseResult<ParserTValue>;

        Parsers parsers;

        List(const std::string& _name, const Parsers& _parsers) : ParserOperation<ParserTValue>(_name), parsers(_parsers) { }

        Parser<ParserTValue> GetParser() const override
        {
            return ParserT(this->name, [parsers = parsers](const Position& _pos, StringStream& _stream)
                {
                    auto results = std::apply([&](auto &&... _args) { return std::make_tuple(_args.Parse(_stream)...); }, parsers);
                    return ParserTResult(sizeof...(Ts) == 0 ? _pos : std::get<0>(results).position, results);
                });
        }
    };

    template<typename First, typename Second>
    List<First, Second> operator&(const Parser<First>& _first, const Parser<Second>& _second)
    {
        return List<First, Second>("(" + _first.GetName() + " & " + _second.GetName() + ")", { _first, _second });
    }

    template<typename... Head, typename Appendage>
    List<Head..., Appendage> operator&(const List<Head...>& _head, const Parser<Appendage>& _appendage)
    {
        auto list = List<Head..., Appendage>(_head.name, std::tuple_cat(_head.parsers, std::make_tuple(_appendage)));
        list.name.replace(list.name.size() - 1, 1, " & " + _appendage.GetName() + ")");
        return list;
    }

    template<typename First, typename Second>
    List<First, Second> operator&(const ParserOperation<First>& _first, const Parser<Second>& _second) { return Parser(_first) & _second; }

    template<typename First, typename Second>
    List<First, Second> operator&(const Parser<First>& _first, const ParserOperation<Second>& _second) { return _first & Parser(_second); }

    template<typename First, typename Second>
    List<First, Second> operator&(const ParserOperation<First>& _first, const ParserOperation<Second>& _second) { return Parser(_first) & Parser(_second); }
#pragma endregion

#pragma region Choice
    template<typename T>
    struct Choice : public ParserOperation<T>
    {
        std::vector<Parser<T>> parsers;

        Choice(const std::string& _name, const std::vector<Parser<T>>& _parsers) : ParserOperation<T>(_name), parsers(_parsers) { }

        Parser<T> GetParser() const override
        {
            assert(parsers.size() >= 2 && "Choice expects at least 2 options");

            return Parser<T>(this->name, [parsers = parsers](const Position& _pos, StringStream& _stream)
                {
                    size_t streamStart = _stream.GetOffset(), greatestLength = 0;
                    std::optional<ParseResult<T>> result;
                    std::vector<ParseError> errors;

                    for (auto parser : parsers)
                    {
                        try
                        {
                            auto parseResult = parser.Parse(_stream);
                            auto length = _stream.GetOffset() - streamStart;

                            if (!result.has_value() || length > greatestLength)
                            {
                                result = parseResult;
                                greatestLength = length;
                                errors.clear(); //We put this here to save memory
                            }
                        }
                        catch (const ParseError& e)
                        {
                            if (!result.has_value())
                            {
                                auto eLength = _stream.GetOffset(e.GetPosition());
                                auto errorsLength = errors.empty() ? 0 : _stream.GetOffset(errors.back().GetPosition());

                                if (eLength == errorsLength) { errors.push_back(e); }
                                else if (eLength > errorsLength) { errors = { e }; }
                            }
                        }

                        _stream.SetOffset(streamStart);
                    }

                    if (!result.has_value())
                    {
                        if (errors.size() == 1) { throw errors.back(); }
                        else
                        {
                            std::stringstream ss;
                            ss << "Expected one of the following: " << parsers[0].GetName();

                            for (size_t i = 1; i < parsers.size(); i++)
                                ss << ", " << parsers[i].GetName();

                            throw ParseError(_stream.GetPosition(), ss.str());
                        }
                    }

                    _stream.SetOffset(streamStart + greatestLength);
                    return result.value();
                });
        }
    };

    template<typename T>
    Choice<T> operator|(const Parser<T>& _option1, const Parser<T>& _option2)
    {
        return Choice<T>("(" + _option1.GetName() + " | " + _option2.GetName() + ")", std::vector({ _option1, _option2 }));
    }

    template<typename T>
    Choice<T> operator|(const Choice<T>& _choices, const Parser<T>& _additional)
    {
        auto choice = _choices;
        choice.name.replace(choice.name.size() - 1, 1, " | " + _additional.GetName() + ")");
        choice.parsers.push_back(_additional);
        return choice;
    }

    template<typename T>
    Choice<T> operator|(const ParserOperation<T>& _option1, const Parser<T>& _option2) { return Parser(_option1) | _option2; }

    template<typename T>
    Choice<T> operator|(const Parser<T>& _option1, const ParserOperation<T>& _option2) { return _option1 | Parser(_option2); }

    template<typename T>
    Choice<T> operator|(const ParserOperation<T>& _option1, const ParserOperation<T>& _option2) { return Parser(_option1) | Parser(_option2); }
#pragma endregion

#pragma region Variant
    template<typename... Ts>
    class VariantValue
    {
    public:
        typedef std::variant<Ts...> Type;

    private:
        Type value;

    public:
        VariantValue(const Type& _value) : value(_value) { }
        VariantValue() : value() { }

        template<size_t Idx>
        bool Is() const { return value.index() == Idx; }

        template<size_t Idx>
        const std::variant_alternative_t<Idx, Type>& Get() const { return std::get<Idx>(value); }

        template<size_t Idx>
        static VariantValue Create(const std::variant_alternative_t<Idx, Type>& _value) { return VariantValue(Type(std::in_place_index<Idx>, _value)); }
    };

    template<typename... Ts>
    struct Variant : public ParserOperation<VariantValue<Ts...>>
    {
        static constexpr size_t Size = sizeof...(Ts);
        using Parsers = std::tuple<Parser<Ts>...>;
        using MappedParserValue = VariantValue<Ts...>;
        using MappedParsers = std::vector<Parser<MappedParserValue>>;

        Parsers parsers;

        Variant(const std::string& _name, Parsers _parsers) : ParserOperation<MappedParserValue>(_name), parsers(_parsers) { }

        Parser<MappedParserValue> GetParser() const override
        {
            MappedParsers mappedParsers(Size);
            PopulateMappedParsers<Size - 1>(mappedParsers);
            return Choice<MappedParserValue>(this->name, mappedParsers).GetParser();
        }

        template <size_t Idx>
        void PopulateMappedParsers(MappedParsers& _parsers) const
        {
            using ArgType = std::variant_alternative_t<Idx, typename MappedParserValue::Type>;

            _parsers[Idx] = std::get<Idx>(parsers).template Map<MappedParserValue>([](const ParseResult<ArgType>& _value)
                {
                    return MappedParserValue::template Create<Idx>(_value.value);
                });

            if constexpr (Idx != 0)
                PopulateMappedParsers<Idx - 1>(_parsers);
        }
    };

    template<typename Opt1, typename Opt2>
    Variant<Opt1, Opt2> operator||(const Parser<Opt1>& _option1, const Parser<Opt2>& _option2)
    {
        return Variant<Opt1, Opt2>("(" + _option1.GetName() + " | " + _option2.GetName() + ")", { _option1, _option2 });
    }

    template<typename... Initial, typename Extra>
    Variant<Initial..., Extra> operator||(Variant<Initial...> _initial, const Parser<Extra>& _extra)
    {
        auto variant = Variant<Initial..., Extra>(_initial.name, std::tuple_cat(_initial.parsers, std::make_tuple(_extra)));
        variant.name.replace(variant.name.size() - 1, 1, " | " + _extra.GetName() + ")");
        return variant;
    }

    template<typename Opt1, typename Opt2>
    Variant<Opt1, Opt2> operator||(const ParserOperation<Opt1>& _option1, const Parser<Opt2>& _option2) { return Parser(_option1) || _option2; }

    template<typename Opt1, typename Opt2>
    Variant<Opt1, Opt2> operator||(const Parser<Opt1>& _option1, const ParserOperation<Opt2>& _option2) { return _option1 || Parser(_option2); }

    template<typename Opt1, typename Opt2>
    Variant<Opt1, Opt2> operator||(const ParserOperation<Opt1>& _option1, const ParserOperation<Opt2>& _option2) { return Parser(_option1) || Parser(_option2); }
#pragma endregion

#pragma region Sum
    template<typename T>
    struct Sum : public ParserOperation<std::vector<ParseResult<T>>>
    {
        using Parsers = std::vector<Parser<T>>;
        using ParserTValue = std::vector<ParseResult<T>>;
        using ParserTResult = ParseResult<ParserTValue>;
        using ParserT = Parser<ParserTValue>;

        Parsers parsers;

        Sum(const std::string& _name, const Parsers& _parsers) : ParserOperation<ParserTValue>(_name), parsers(_parsers) { }

        ParserT GetParser() const override
        {
            return ParserT(this->name, [parsers = parsers](const Position& _pos, StringStream& _stream)
                {
                    ParserTValue values;

                    for (auto parser : parsers)
                        values.push_back(parser.Parse(_stream));

                    Position position = values.empty() ? _pos : values[0].position; //this is separated from the return statement because of the unknown order of argument evaluation 
                    return ParserTResult(position, std::move(values));
                });
        }
    };

    template<typename T>
    Sum<T> operator+(const Parser<T>& _lhs, const Parser<T>& _rhs)
    {
        return Sum<T>("(" + _lhs.GetName() + " + " + _rhs.GetName() + ")", std::vector({ _lhs, _rhs }));
    }

    template<typename T>
    Sum<T> operator+(const Sum<T>& _lhs, const Parser<T>& _rhs)
    {
        auto sum = _lhs;
        sum.name.replace(sum.name.size() - 1, 1, " + " + _rhs.GetName() + ")");
        sum.parsers.push_back(_rhs);
        return sum;
    }

    template<typename T>
    Sum<T> operator+(const ParserOperation<T>& _lhs, const Parser<T>& _rhs) { return Parser(_lhs) + _rhs; }

    template<typename T>
    Sum<T> operator+(const Parser<T>& _lhs, const ParserOperation<T>& _rhs) { return _lhs + Parser(_rhs); }

    template<typename T>
    Sum<T> operator+(const ParserOperation<T>& _lhs, const ParserOperation<T>& _rhs) { return Parser(_lhs) + Parser(_rhs); }
#pragma endregion

#pragma region BinopChain
    enum class BinopAssociativity { RIGHT, LEFT, NONE };

    template<typename ID>
    struct Binop
    {
        ID id;
        size_t precedence;
        BinopAssociativity associatvity;
    };

    template<typename T, typename B>
    using BinopChainCombiner = std::function<ParseResult<T>(const ParseResult<T>& _lhs, const ParseResult<Binop<B>>& _op, const ParseResult<T>& _rhs)>;

    template<typename T, typename B>
    ParseResult<T> BinopChainFunc(StringStream& _stream, const Parser<T>& _atom, const Parser<Binop<B>>& _op, BinopChainCombiner<T, B> _bcc, size_t _curPrecedence)
    {
        auto chain = _atom.Parse(_stream);

        while (true)
        {
            auto streamStart = _stream.GetOffset();
            ParseResult<Binop<B>> op;

            try { op = _op.Parse(_stream); }
            catch (const ParseError& e) { break; }

            if (op.value.precedence < _curPrecedence)
            {
                _stream.SetOffset(streamStart);
                break;
            }

            auto rhs = BinopChainFunc(_stream, _atom, _op, _bcc, op.value.precedence + (op.value.associatvity == BinopAssociativity::RIGHT ? 0 : 1));
            chain = _bcc(chain, op, rhs);
        }

        return chain;
    }

    template<typename T, typename B>
    Parser<T> BinopChain(const std::string& _name, const Parser<T>& _atom, const Parser<Binop<B>>& _op, BinopChainCombiner<T, B> _bcc)
    {
        return Parser<T>(_name, [=](const Position& _pos, StringStream& _stream) { return BinopChainFunc(_stream, _atom, _op, _bcc, 0); });
    }
#pragma endregion

#pragma region Fold
    template<typename T, typename F>
    Parser<F> FoldL(const std::string& _name, const Parser<T>& _parser, const F& _initial, std::function<void(F&, const ParseResult<T>&)> _func)
    {
        return Parser<F>(_name, [=](const Position& _pos, StringStream& _stream)
            {
                auto value = _initial;

                while (true)
                {
                    std::optional<ParseResult<T>> result;

                    try { result = _parser.Parse(_stream); }
                    catch (const ParseError& e) { break; }

                    _func(value, result.value());
                }

                return ParseResult<F>(_pos, value);
            });
    }

    template<typename T>
    Parser<T> FoldL(const std::string& _name, const Parser<T>& _parser, const T& _initial, std::function<void(T&, const ParseResult<T>&)> _func) { return FoldL<T, T>(_name, _parser, _initial, _func); }
#pragma endregion

    template<typename T>
    class Recursive : public Parser<T>
    {
        std::shared_ptr<Parser<T>> parser;

    public:
        Recursive(const std::string& _name) : Parser<T>(_name, [=](const Position& _pos, StringStream& _stream) { return parser->Parse(_stream); }) {}

        void Set(const Parser<T>& _parser) { parser.reset(new Parser<T>(_parser)); }
    };

    template<typename T>
    Parser<T> Value(const std::string& _name, const T& _value)
    {
        return Parser<T>(_name, [=](const Position& _pos, StringStream& _stream) { return ParseResult<T>(_pos, _value); });
    }

    template<typename T, typename S>
    QuantifiedParser<T> Separated(const std::string& _name, const Parser<T>& _parser, const Parser<S>& _sep, size_t _min = 0, size_t _max = SIZE_T_MAX)
    {
        assert(_max >= _min && "_max must be at least _min");

        if (_max == 0) { return Quantified(_name, _parser, 0, 0); }
        else if (_max == 1) { return ZeroOrOne(_name, _parser); }
        else
        {
            size_t min = _min == 0 ? 0 : _min - 1;
            Parser tempParser = _parser & Quantified("TAIL(" + _sep.GetName() + ", " + _parser.GetName() + ")", _sep >> _parser, min, _max - 1);
            Parser parser = tempParser.template Map<QuantifiedValue<T>>([](auto _result)
                {
                    QuantifiedValue<T> result = { std::get<0>(_result.value) };
                    auto& tail = std::get<1>(_result.value).value;
                    result.insert(result.end(), tail.begin(), tail.end());
                    return result;
                });

            if (_min == 0)
                parser = Optional(parser).template Map<QuantifiedValue<T>>([](auto _result) { return _result.value.has_value() ? _result.value.value() : QuantifiedValue<T>(); });

            return Parser(_name, parser);
        }
    }

    template<typename T>
    Parser<ParseError> Not(const Parser<T>& _parser)
    {
        return Parser<ParseError>("!" + _parser.GetName(), [=](const Position& _pos, StringStream& _stream)
            {
                try
                {
                    _parser.Parse(_stream);
                    throw ParseError(_stream.GetPosition(), "Expected " + _parser.GetName() + " to fail");
                }
                catch (const ParseError& e) { return ParseResult<ParseError>(e.GetPosition(), e); }
            });
    }

    template<typename T>
    Parser<T> LookAhead(const Parser<T>& _parser)
    {
        return Parser<T>("LOOKAHEAD(" + _parser.GetName() + ")", [=](const Position& _pos, StringStream& _stream)
            {
                auto result = _parser.Parse(_stream);

                _stream.SetPosition(_pos);
                return result;
            });
    }

    template<typename Open, typename T, typename Close>
    Parser<T> Between(const std::string& _name, const Parser<Open>& _open, const Parser<T>& _parser, const Parser<Close>& _close)
    {
        return Parser<T>(_name, [=](const Position& _pos, StringStream& _stream)
            {
                _open.Parse(_stream);
                auto result = _parser.Parse(_stream);
                _close.Parse(_stream);
                return result;
            });
    }

    template<typename P, typename T>
    Parser<T> Prefixed(const std::string& _name, const Parser<P>& _prefix, const Parser<T>& _parser)
    {
        return Parser<T>(_name, [=](const Position& _pos, StringStream& _stream)
            {
                _prefix.Parse(_stream);
                return _parser.Parse(_stream);
            });
    }

    template<typename T, typename S>
    Parser<T> Suffixed(const std::string& _name, const Parser<T>& _parser, const Parser<S>& _suffix)
    {
        return Parser<T>(_name, [=](const Position& _pos, StringStream& _stream)
            {
                auto result = _parser.Parse(_stream);
                _suffix.Parse(_stream);
                return result;
            });
    }

    template<typename Keep, typename Discard>
    Parser<Keep> operator<<(const Parser<Keep>& _keep, const Parser<Discard>& _discard) { return Suffixed("(" + _keep.GetName() + " << " + _discard.GetName() + ")", _keep, _discard); }

    template<typename Discard, typename Keep>
    Parser<Keep> operator>>(const Parser<Discard>& _discard, const Parser<Keep>& _keep) { return Prefixed("(" + _discard.GetName() + " >> " + _keep.GetName() + ")", _discard, _keep); }

    Parser<std::string> Terminal(const std::string& _name, const Lexer::PatternID& _patternID, std::optional<std::string> _value = std::nullopt);
    Parser<std::string> Terminal(const std::string& _name, const Regex& _regex, std::optional<std::string> _value = std::nullopt);
    Parser<std::string> Chars(const std::string& _name, std::optional<std::string> _value = std::nullopt);
    Parser<std::string> Letters(const std::string& _name, std::optional<std::string> _value = std::nullopt);
    Parser<std::string> Digits(const std::string& _name, std::optional<std::string> _value = std::nullopt);
    Parser<std::string> AlphaNums(const std::string& _name, std::optional<std::string> _value = std::nullopt);
    Parser<std::string> Whitespace(const std::string& _name, std::optional<std::string> _value = std::nullopt);
    Parser<char> Char(const std::string& _name, std::optional<char> _value = std::nullopt);
    Parser<std::monostate> EOS(const std::string& _name);
    Parser<std::monostate> Error(const std::string& _name, const std::string& _message);
}