from read import readInput
from write import writeOutput
from ctypes import *
import os

from host import GO

def ctype_matrix(matrix):
    matrix1d = [item for sublist in matrix for item in sublist]
    c_array = (c_int * len(matrix1d))(*matrix1d) 
    return c_array

class MyPlayer():
    def get_input(self, go, piece_type):
        '''
        Get one input.

        :param go: Go instance.
        :param piece_type: 1('X') or 2('O').
        :return: (row, column) coordinate of input.
        '''
        
        c_board = ctype_matrix(go.board)
        c_previous_board = ctype_matrix(go.previous_board)
        
        # 这里默认的是相对路径，如果报错，麻烦助教手动改为绝对路径谢谢
        onnxPath = os.path.abspath("data/model9.onnx")
        soPath = os.path.abspath("/home/xuyisen/project/Go_game/KataGoLike/cpp/build/libget_input.so")
        logPath = os.path.abspath("data")
        timeLimit = 12   #单位：秒
        
        # use ctypes to call the c++ function
        lib = CDLL(soPath)
        move = lib.get_input(c_board, c_previous_board, piece_type, onnxPath.encode(), timeLimit, logPath.encode())
        
        if move == -1:
            return "PASS"
        else:
            return (move // go.size, move % go.size)
        

if __name__ == "__main__":
    N = 5
    piece_type, previous_board, board = readInput(N)
    go = GO(N)
    go.set_board(piece_type, previous_board, board)
    player = MyPlayer()
    action = player.get_input(go, piece_type)
    writeOutput(action)