from ctypes import *

from PIL import Image


class NevenFace(Structure):
    _fields_ = [
        ("confidence", c_float),
        ("midpointx", c_float),
        ("midpointy", c_float),
        ("eyedist", c_float),
    ]


nevenbmd = "/usr/share/neven/bmd/RFFprec_501.bmd"
libneven = CDLL('libneven.so')
libneven.neven_create.argtypes = [c_void_p, c_int, c_int, c_int]
libneven.neven_create.restype = c_void_p
libneven.neven_detect.argtypes = [c_void_p, c_char_p]
libneven.neven_get_face.argtypes = [c_void_p, POINTER(NevenFace), c_int]
libneven.neven_destroy.argtypes = [c_void_p]


class Neven(object):
    def __init__(self, width, height, max_faces=32):
        self.env = libneven.neven_create(nevenbmd, width, height, max_faces)

    def detect_faces(self, buf):
        faces = []
        for idx in range(libneven.neven_detect(self.env, buf)):
            face = NevenFace()
            libneven.neven_get_face(self.env, byref(face), idx)
            faces.append(face)
        return faces

    def __del__(self):
        libneven.neven_destroy(self.env)


def detect_faces_from_image(image):
    img = image.convert('L')
    width, height = img.size
    if max(width, height) > 600:
        scale = max(width, height) / 600.0
        img.thumbnail((int(width / scale), int(height / scale)), Image.BILINEAR)
    else:
        scale = 1.0

    neven = Neven(*img.size)
    faces = neven.detect_faces(img.tostring())
    for face in faces:
        face.midpointx *= scale
        face.midpointy *= scale
        face.eyedist *= scale
    return faces


def detect_faces_from_filename(filename):
    return detect_faces_from_image(Image.open(filename))
