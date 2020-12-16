import ctypes
import os

libname = os.path.abspath(
    os.path.join("bin", "libwsianon.so"))

print(libname)

cwd = os.getcwd()
_wsi_anonymizer = ctypes.CDLL(libname)
#_wsi_anonymizer.check_file_format.argtypes = (ctypes.c_char_p)
#_wsi_anonymizer.anonymize_wsi.argtypes = (ctypes.c_char_p)

def test_function():
    print("in function")
    global _wsi_anonymizer
    _wsi_anonymizer.test_function()
    print("leave function")

def check_file_format(filename):
    global _wsi_anonymizer
    length = len(filename)
    filename_array = ctypes.c_char * length
    result = _wsi_anonymizer.check_file_format(filename_array(*filename))
    return str(result)

def anonymize_wsi(filename, new_label_name, disbale_unlinking=False, disable_inplace=True):
    global _wsi_anonymizer
    length_filename = len(filename)
    length_labelname = len(new_label_name)
    filename_array = ctypes.c_char * length_filename
    labelname_array = ctypes.c_char * length_labelname
    result = _wsi_anonymizer.anonymize_wsi(filename_array(*filename),
        labelname_array(*new_label_name),
        ctypes.c_bool(unlink_directory))
    return str(result)