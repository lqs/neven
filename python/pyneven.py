from ctypes import *

class NevenFace(Structure):
    _fields_ = [
        ("confidence", c_float),
        ("midpointx", c_float),
        ("midpointy", c_float),
        ("eyedist", c_float),
    ]

libneven = CDLL('libneven.so')
libneven.neven_create.argtypes = [c_int, c_int, c_int]
libneven.neven_detect.argtypes = [c_void_p, c_char_p]
libneven.neven_get_face.argtypes = [c_void_p, POINTER(NevenFace), c_int]
libneven.neven_destroy.argtypes = [c_void_p]

class Neven:
    def __init__(self, width, height, max_faces = 32):
        self.env = libneven.neven_create(width, height, max_faces)
    def detect_faces(self, buf):
        faces = []
        for idx in range(libneven.neven_detect(self.env, buf)):
            face = NevenFace()
            libneven.neven_get_face(self.env, byref(face), idx)
            faces.append(face)
        return faces
    def __del__(self):
        libneven.neven_destroy(self.env)

