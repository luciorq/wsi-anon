import ctypes
import os
from enum import Enum

class Vendor(Enum):
    Aperio = 0
    Hamamatsu = 1
    Mirax = 2
    Unknown = 3
    Invalid = 4

libname = os.path.abspath(os.path.join("bin", "libwsianon.so"))
_wsi_anonymizer = ctypes.cdll.LoadLibrary(libname)

def check_file_format(filename):
    global _wsi_anonymizer
    _wsi_anonymizer.argtypes = [ctypes.c_char_p]
    c_filename = filename.encode('utf-8')
    result = _wsi_anonymizer.check_file_format(c_filename)
    return Vendor(result)

def anonymize_wsi(filename, new_label_name, keep_macro_image=False, disable_unlinking=False, do_inplace=False):
    global _wsi_anonymizer
    _wsi_anonymizer.anonymize_wsi.argtypes = [ctypes.c_char_p, ctypes.c_char_p, ctypes.c_bool, ctypes.c_bool, ctypes.c_bool]
    _wsi_anonymizer.anonymize_wsi.restype = ctypes.c_char_p
    c_filename = filename.encode('utf-8')
    c_new_label_name = new_label_name.encode('utf-8')
    result =_wsi_anonymizer.anonymize_wsi(c_filename, c_new_label_name, keep_macro_image, disable_unlinking, do_inplace)
    return ctypes.c_char_p(result).value.decode('utf-8')