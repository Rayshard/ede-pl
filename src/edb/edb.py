import subprocess
import sys, threading
from subprocess import Popen
from queue import Queue
from typing import Callable, Optional, List
import os
import platform
from subprocess import PIPE, Popen
from typing import List, Optional
import threading
import tempfile
from queue import Queue
from PyQt5 import QtCore, QtWidgets

# To regenerate gui.py, run 'pyuic5 src/edb/gui.ui -o src/edb/gui.py'
from .gui import Ui_MainWindow

EVM_PATH = "bin/evm" + (".exe" if platform.system() == "Windows" else "")


class Debugger:
    def __init__(self, prog_path: str, prog_args: List[str], start_callback: Callable[[], None], stop_callback: Callable[[], None]) -> None:
        self.__running = False

        self.prog_path: str = prog_path
        self.prog_args: str = prog_args
        self.program: Optional[Popen] = None
        self.stdout: Queue[str] = Queue[str]()
        self.stderr: Queue[str] = Queue[str]()
        self.stdin: Queue[str] = Queue[str]()

        self.dg_to_evm_id: str = ""
        self.evm_to_dg_id: str = ""

        self.communicationThread = threading.Thread(target=self.__Communicate)
        self.readerThread = threading.Thread(target=self.__Read)
        self.writerThread = threading.Thread(target=self.__Write)

        self.readQueue: Queue[str] = Queue[str]()
        self.writeQueue: Queue[str] = Queue[str]()

        self.start_callback = start_callback
        self.stop_callback = stop_callback

    def Start(self):
        if self.__running:
            return

        self.__running = True

        self.dg_to_evm_id = tempfile.mkstemp()[1]
        self.evm_to_dg_id = tempfile.mkstemp()[1]
        self.program = Popen([EVM_PATH, "run", "--debugger", self.dg_to_evm_id, self.evm_to_dg_id, self.prog_path] + self.prog_args, stdin=PIPE, stdout=PIPE, stderr=PIPE)

        self.readerThread.start()
        self.writerThread.start()
        self.communicationThread.start()
        self.start_callback()

    def Stop(self):
        if not self.__running:
            return

        self.__running = False
        self.communicationThread.join()
        self.readerThread.join()
        self.writerThread.join()
        self.__CleanUp()

    def IsRunning(self):
        return self.__running or self.communicationThread.is_alive() or self.readerThread.is_alive() or self.writerThread.is_alive()

    def __CleanUp(self):
        try:
            os.remove(self.dg_to_evm_id)
            os.remove(self.evm_to_dg_id)
        except Exception as e:
            print("Not cleaned up: " + str(e))

    def __Communicate(self):
        def ReadSTDOut():
            while self.__running and self.program.poll() is None:
                data = self.program.stdout.readline()
                if len(data) != 0:
                    self.stdout.put(data.decode("utf-8"))

        def ReadSTDErr():
            while self.__running and self.program.poll() is None:
                data = self.program.stderr.readline()
                if len(data) != 0:
                    self.stderr.put(data.decode("utf-8"))

        # Start stdout, stderr, and stdin threads
        stdoutThread = threading.Thread(target=ReadSTDOut)
        stderrThread = threading.Thread(target=ReadSTDErr)
        stdoutThread.start()
        stderrThread.start()

        # Block until program terminates or debugger is no longer running
        while self.program.poll() is None:
            if not self.__running:
                self.program.terminate()
                break

        # Add any remaining stdout or stderr left in the program's buffer to the appropriate queues
        out, err = self.program.communicate()

        if len(out) != 0:
            self.stdout.put(out.decode("utf-8"))

        if len(err) != 0:
            self.stderr.put(err.decode("utf-8"))

        # Close all threads
        self.__running = False
        stdoutThread.join()
        stderrThread.join()
        self.readerThread.join()
        self.writerThread.join()
        self.__CleanUp()
        self.stop_callback()

    def __Read(self):
        with open(self.evm_to_dg_id) as r:
            while self.__running:
                header = r.readline().strip()
                if not header:
                    continue
                elif not header.isnumeric():
                    print("Debug data header is invalid: " + header)
                    continue

                data = r.read(int(header))
                self.readQueue.put(data)
                print(f"Python reading: {len(data)}, {data}")

    def __Write(self):
        with open(self.dg_to_evm_id, "w") as w:
            while self.__running:
                while not self.writeQueue.empty():
                    data = self.writeQueue.get()
                    w.write(str(len(data)) + "\n")
                    w.write(data + "\n")

class AppConsoleThread(QtCore.QThread):

    stdout_received = QtCore.pyqtSignal(str)
    stderr_received = QtCore.pyqtSignal(str)

    def __init__(self, debugger: Debugger):
        QtCore.QThread.__init__(self)
        self.db = debugger

    def run(self):
        while self.db.IsRunning() or not self.db.stdout.empty() or not self.db.stderr.empty():
            while not self.db.stdout.empty():
                self.stdout_received.emit(self.db.stdout.get())

            while not self.db.stderr.empty():
                self.stderr_received.emit(self.db.stderr.get())

class Application(QtWidgets.QMainWindow, Ui_MainWindow):
    def __init__(self, program_path: str, program_args: List[str]) -> None:
        super(Application, self).__init__(None)
        self.setupUi(self)

        self.prog_path = program_path
        self.prog_args = program_args

        self.db = Debugger(self.prog_path, self.prog_args, self.onDebuggerStarted, self.onDebuggerStopped)
        self.console_thread = AppConsoleThread(self.db)
        self.console_thread.stdout_received.connect(self.onConsoleDataReceived)
        self.console_thread.stderr_received.connect(self.onConsoleDataReceived)

        # Init status bar
        self.statusbar_label = QtWidgets.QLabel()
        self.statusbar.addPermanentWidget(self.statusbar_label)
        

    def onLoaded(self):
        self.db.Start()
        
    def onDebuggerStarted(self):
        self.statusbar_label.setText("Running " + self.prog_path)
        self.console_thread.start()

    def onDebuggerStopped(self):
        self.statusbar_label.setText("Finished " + self.prog_path)

    def closeEvent(self, event):
        self.db.Stop()

        while self.console_thread.isRunning():
            pass

        print("Window closed successfully!")
        event.accept()
        
    def onConsoleDataReceived(self, data: str):
        self.plainTextEdit_Console.insertPlainText(data)

def main():
    prog_path = sys.argv[1] if len(sys.argv) > 1 else ""
    prog_args = sys.argv[2:] if len(sys.argv) > 2 else []

    qapp = QtWidgets.QApplication(sys.argv)  # Initialize QT

    # Run application
    app = Application(prog_path, prog_args)
    app.show()

    # Used to call callback once window is first shown
    t = QtCore.QTimer()
    t.singleShot(0, app.onLoaded)

    sys.exit(qapp.exec_())
