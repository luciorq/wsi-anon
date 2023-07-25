import ctypes
import os
import threading

from enum import Enum

class Vendor(Enum):
    APERIO = 0
    HAMAMATSU = 1
    MIRAX = 2
    VENTANA = 3
    ISYNTAX = 4
    PHILIPS_TIFF = 5
    UNKNOWN = 6
    INVALID = 7

class MetadataAttribute(ctypes.Structure):
    _fields_ = [("key", ctypes.c_char_p),
                ("value", ctypes.c_char_p)]

class Metadata(ctypes.Structure):
    _fields_ = [("metadataAttributes", ctypes.POINTER(MetadataAttribute)),
                ("length", ctypes.c_size_t)]

# TODO: extend the fields by members of struct
class WSIData(ctypes.Structure):
    _fields_ = [("format", ctypes.c_int8),
                ("filename", ctypes.c_char_p),
                ("metadata", ctypes.POINTER(Metadata))]

lock = threading.Lock()

libname = os.path.abspath(os.path.join("bin", "libwsianon.so"))
_wsi_anonymizer = ctypes.cdll.LoadLibrary(libname)

def get_wsi_data(filename):
    global _wsi_anonymizer
    _wsi_anonymizer.get_wsi_data.argtypes = [ctypes.c_char_p]
    _wsi_anonymizer.get_wsi_data.restype = ctypes.c_void_p

    c_filename = filename.encode('utf-8')

    result = WSIData.from_address(_wsi_anonymizer.get_wsi_data((ctypes.c_char_p(c_filename))))
    return Vendor(result.format)

def anonymize_wsi(filename, new_label_name, keep_macro_image=False, disable_unlinking=False, do_inplace=False):
    global _wsi_anonymizer
    _wsi_anonymizer.anonymize_wsi.argtypes = [ctypes.c_char_p, ctypes.c_char_p, ctypes.c_bool, ctypes.c_bool, ctypes.c_bool]
    _wsi_anonymizer.anonymize_wsi.restype = ctypes.c_void_p

    filename = str(filename).replace(" ", "\\ ") # escape whitespaces if exist
    c_filename = filename.encode('utf-8')
    c_new_label_name = new_label_name.encode('utf-8')

    result = -1
    with lock:
        result = _wsi_anonymizer.anonymize_wsi(
            c_filename, 
            c_new_label_name,
            keep_macro_image, 
            disable_unlinking, 
            do_inplace
        )
    
    return result