import ctypes
import os
import threading

from enum import Enum

class Vendor(Enum):
    Aperio = 0
    Hamamatsu = 1
    Mirax = 2
    Ventana = 3
    Isyntax = 4
    Unknown = 5
    Invalid = 6

lock = threading.Lock()

libname = os.path.abspath(os.path.join("bin", "libwsianon.so"))
_wsi_anonymizer = ctypes.cdll.LoadLibrary(libname)

def check_file_format(filename):
    global _wsi_anonymizer
    # replace check_file_format with get_wsi_data function in wsi-anonymizer.c
    _wsi_anonymizer.check_file_format.argtypes = [ctypes.c_char_p]

    c_filename = filename.encode('utf-8')

    # replace check_file_format with get_wsi_data function in wsi-anonymizer.c
    result = _wsi_anonymizer.check_file_format(ctypes.c_char_p(c_filename))
    return Vendor(result)

def anonymize_wsi(filename, new_filename, pseudonym_metadata='X', keep_macro_image=False, disable_unlinking=False, do_inplace=False):
    global _wsi_anonymizer
    _wsi_anonymizer.anonymize_wsi.argtypes = [ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_bool, ctypes.c_bool, ctypes.c_bool]
    _wsi_anonymizer.anonymize_wsi.restype = ctypes.c_void_p

    filename = str(filename).replace(" ", "\\ ") # escape whitespaces if exist
    c_filename = filename.encode('utf-8')
    c_new_filename = new_filename.encode('utf-8')
    c_pseudonym_metadata = pseudonym_metadata.encode('utf-8')

    result = -1
    with lock:
        result = _wsi_anonymizer.anonymize_wsi(
            c_filename, 
            c_new_filename,
            c_pseudonym_metadata,
            keep_macro_image, 
            disable_unlinking, 
            do_inplace
        )
    
    return result