import ctypes
import os
from enum import Enum

class Vendor(Enum):
    Aperio = 1
    Hamamatsu = 2
    Mirax = 3
    Unknown = 4
    Invalid = 5

libname = os.path.abspath(os.path.join("bin", "libwsianon.so"))
_wsi_anonymizer = ctypes.cdll.LoadLibrary(libname)

def check_file_format(filename):
    global _wsi_anonymizer
    _wsi_anonymizer.argtypes = [ctypes.c_char_p]
    result = _wsi_anonymizer.check_file_format(filename)
    return Vendor(result)

def anonymize_wsi(filename, new_label_name, keep_macro_image=False, disable_unlinking=False, do_inplace=False):
    global _wsi_anonymizer
    _wsi_anonymizer.argtypes = [ctypes.c_char_p, ctypes.c_char_p, ctypes.c_bool, ctypes.c_bool, ctypes.c_bool]
    _wsi_anonymizer.restype = [ctypes.c_char_p]
    result = _wsi_anonymizer.anonymize_wsi(filename, new_label_name, keep_macro_image, disable_unlinking, do_inplace)
    return result