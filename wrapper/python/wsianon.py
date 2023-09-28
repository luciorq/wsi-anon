import ctypes
import os
import threading
import platform

try: 
    from model.model import *
except: 
    from .model.model import *
    
lock = threading.Lock()

def _load_library():
    '''
    loads library depending on operating system. This is currently only implemented only Windows and Linux
    '''
    if platform.system() == 'Linux':
        try:
            return ctypes.cdll.LoadLibrary('libwsianon.so')
        except FileNotFoundError:
            raise ModuleNotFoundError(
                "Could not locate libwsianon.so. "
                "Please make sure that the shared "
                "library is created and placed under "
                "usr/lib/ by running make install. "
            )
    elif platform.system() == 'Windows':
        try:
            print("ENTERS AND USES THIS")
            return ctypes.WinDLL("libwsianon.dll")
        except FileNotFoundError:
            raise ModuleNotFoundError(
                "Could not locate libwsianon.dll.  "
                "Please make sure that the DLL "
                "is created and placed under "
                "C:\Windows\Systems32. "
            )
    else:
        raise ModuleNotFoundError(
                "Could not locate shared library or DLL.  "
                "Please make sure you are running under Linux or Windows.  "
            )

_wsi_anonymizer = _load_library()
print(_wsi_anonymizer)

def get_wsi_data(filename):
    '''
    gets all necessary WSI data from slide
    '''
    global _wsi_anonymizer
    _wsi_anonymizer.get_wsi_data.argtypes = [ctypes.c_char_p]
    _wsi_anonymizer.get_wsi_data.restype = ctypes.c_void_p

    c_filename = filename.encode('utf-8')

    wsi_data = WSIData.from_address(_wsi_anonymizer.get_wsi_data((ctypes.c_char_p(c_filename))))
    return wsi_data

def anonymize_wsi(filename, new_label_name, keep_macro_image=False, disable_unlinking=False, do_inplace=False):
    '''
    performs anonymization on slide
    '''
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