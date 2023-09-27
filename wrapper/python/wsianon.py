import ctypes
import os
import threading

try: 
    from model.model import *
except: 
    from .model.model import *
    
lock = threading.Lock()

def search_shared_lib():
    '''
    recursively searches through all directories in case shared lib is not in root directory
    IMPORTANT: libwsianon.so needs to be created prior
    '''
    while(True):
        for root, dirs, files in os.walk("."):
            if('bin' in dirs):
                for root, _, files in os.walk(os.path.join(".", "bin")):
                    if 'libwsianon.so' in files:
                        return os.path.join(os.path.abspath(root), "libwsianon.so")
            if('bin' in root):
                for file in files:
                        if(file == 'libwsianon.so'):
                            return os.path.join(os.path.abspath(root), "libwsianon.so")
        os.chdir('..')

try:
    _wsi_anonymizer = ctypes.cdll.LoadLibrary(os.path.abspath(os.path.join("bin", "libwsianon.so")))
except:
    _wsi_anonymizer = ctypes.cdll.LoadLibrary(search_shared_lib()) 

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