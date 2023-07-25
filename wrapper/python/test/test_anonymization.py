import os
import pathlib
import shutil
import threading
import time
import pytest
import openslide
import tiffslide

from ..wsianon import get_wsi_data, anonymize_wsi
from ..model.model import Vendor

lock = threading.Lock()

@pytest.fixture(scope='session', autouse=True)
def cleanup():
    temporary_files = []
    def add_filename(filename):
        temporary_files.append(filename)

    yield add_filename

    for filename in set(temporary_files):
        remove_file(filename=filename)


def remove_file(filename):
    with lock:
        if filename.endswith(".mrxs"):
            mrxs_path = filename[:len(filename)-5]
            shutil.rmtree(mrxs_path)
        os.remove(filename)


@pytest.fixture(autouse=True)
def wait_between_tests():
    yield
    time.sleep(1)


def wait_until_exists(filename: str, max_wait_in_sec: int):
    while(max_wait_in_sec > 0):
        if pathlib.Path(filename).exists():
            return True
        time.sleep(1)
        max_wait_in_sec -= 1
    return False


@pytest.mark.parametrize(
    "wsi_filename, vendor",
    [
        ("/data/Aperio/CMU-1.svs", Vendor.APERIO),
        ("/data/Hamamatsu/OS-1.ndpi", Vendor.HAMAMATSU),
        ("/data/MIRAX/Mirax2.2-1.mrxs", Vendor.MIRAX),
        #("/data/Ventana/OS-2.bif", Vendor.VENTANA), TODO: check what causes segmentation fault here
    ],
)
def test_format_get_wsi_data(wsi_filename, vendor):
    file_format = get_wsi_data(wsi_filename)
    assert file_format == vendor

# TODO: extend testcases for get_wsi_data function here

@pytest.mark.parametrize(
    "wsi_filepath, original_filename, new_anonyimized_name, file_extension",
    [
        ("/data/Aperio/", "CMU-1", "anon-aperio", "svs"),
    ],
)
def test_anonymize_file_format_tiffslide(cleanup, wsi_filepath, original_filename, new_anonyimized_name, file_extension):
    result_filename = pathlib.Path(wsi_filepath).joinpath(f"{new_anonyimized_name}.{file_extension}")
    if result_filename.exists():
        remove_file(str(result_filename.absolute()))

    wsi_filename = str(pathlib.Path(wsi_filepath).joinpath(f"{original_filename}.{file_extension}").absolute())
    result = anonymize_wsi(wsi_filename, new_anonyimized_name)
    assert result != -1
    
    assert wait_until_exists(str(result_filename), 5)

    try:
        slide = tiffslide.TiffSlide(result_filename)
        associated_images = slide.associated_images
        assert "label" not in associated_images
        
        if "Aperio" in wsi_filepath:
            for property in ["Filename", "User", "Date"]:
                assert all(c == "X" for c in slide.properties[f"aperio.{property}"])
        slide.close()
    except tiffslide.TiffFileError as e:
        assert False
    
    cleanup(str(result_filename.absolute()))


@pytest.mark.parametrize(
    "wsi_filepath, original_filename, new_anonyimized_name, file_extension",
    [
        ("/data/Hamamatsu/", "OS-1", "anon-hamamatsu", "ndpi"),
        #("/data/Ventana/", "OS-2", "anon-ventana", "bif"), # TODO: check what causes segmentation fault here
        #("/data/MIRAX/", "Mirax2.2-1.mrxs", "anon-mirax1", "mrxs"), # TODO: OpenSlide occasionally throws error while initializing
    ],
)
def test_anonymize_file_format_openslide(cleanup, wsi_filepath, original_filename, new_anonyimized_name, file_extension):
    result_filename = pathlib.Path(wsi_filepath).joinpath(f"{new_anonyimized_name}.{file_extension}")
    if result_filename.exists():
        remove_file(str(result_filename.absolute()))

    wsi_filename = str(pathlib.Path(wsi_filepath).joinpath(f"{original_filename}.{file_extension}").absolute())
    result = anonymize_wsi(wsi_filename, new_anonyimized_name)
    assert result != -1
    
    assert wait_until_exists(str(result_filename), 5)

    with openslide.OpenSlide(str(result_filename)) as slide:
        assert "label" not in slide.associated_images

        if "MIRAX" in wsi_filepath:
            assert "macro" not in slide.associated_images
            for property in ["SLIDE_NAME", "PROJECT_NAME", "SLIDE_CREATIONDATETIME"]:
                assert all(c == "X" for c in slide.properties[f"mirax.GENERAL.{property}"])
            assert all(c == "0" for c in slide.properties[f"mirax.GENERAL.SLIDE_ID"])

        if "Ventana" in wsi_filepath:
            for property in ["UnitNumber", "UserName", "BuildDate"]:
                assert all(c == " " for c in slide.properties[f"ventana.{property}"])

        if "Hamamatsu" in wsi_filepath:
            for property in ["DateTime"]:
                assert all(c == "X" for c in slide.properties[f"tiff.{property}"])

    cleanup(str(result_filename.absolute()))


@pytest.mark.parametrize(
    "wsi_filepath, original_filename, new_anonyimized_name, file_extension",
    [
        ("/data/Aperio/", "CMU-1", "anon-aperio2", "svs"),
    ],
)
def test_anonymize_file_format_only_label(cleanup, wsi_filepath, original_filename, new_anonyimized_name, file_extension):
    result_filename = pathlib.Path(wsi_filepath).joinpath(f"{new_anonyimized_name}.{file_extension}")
    if result_filename.exists():
        remove_file(str(result_filename.absolute()))
    
    wsi_filename = str(pathlib.Path(wsi_filepath).joinpath(f"{original_filename}.{file_extension}").absolute())
    result = anonymize_wsi(filename=wsi_filename, new_label_name=new_anonyimized_name, keep_macro_image=True, disable_unlinking=False, do_inplace=False)
    assert result != -1
    
    assert wait_until_exists(str(result_filename), 5)

    with openslide.OpenSlide(str(result_filename)) as slide:
        associated_images = slide.associated_images
        assert "label" not in associated_images
        assert "macro" in associated_images

    cleanup(str(result_filename.absolute()))

@pytest.mark.parametrize(
    "wsi_filepath, original_filename, new_anonyimized_name, file_extension",
    [
        ("/data/Hamamatsu/", "OS-1", "anon-hamamatsu2", "ndpi"),
    ],
)
def test_anonymize_file_format_only_label_hamamatsu(cleanup, wsi_filepath, original_filename, new_anonyimized_name, file_extension):
    result_filename = pathlib.Path(wsi_filepath).joinpath(f"{new_anonyimized_name}.{file_extension}")
    if result_filename.exists():
        remove_file(str(result_filename.absolute()))

    wsi_filename = str(pathlib.Path(wsi_filepath).joinpath(f"{original_filename}.{file_extension}").absolute())
    result = anonymize_wsi(wsi_filename, new_anonyimized_name, keep_macro_image=True)
    assert result != -1

    assert wait_until_exists(str(result_filename), 5)

    with openslide.OpenSlide(str(result_filename)) as slide:
        assert "label" not in slide.associated_images
        assert "macro" not in slide.associated_images

    cleanup(str(result_filename.absolute()))


# Actually both are not working at the moment
@pytest.mark.parametrize(
    "wsi_filepath, original_filename, new_anonyimized_name, file_extension",
    [
        #("/data/Philips iSyntax/", "4399", "anon-philips", "isyntax"),
        #("/data/MIRAX/", "Mirax2.2-1", "anon-mirax2", "mrxs"), 
    ],
)
def test_anonymize_file_format_basic(cleanup, wsi_filepath, original_filename, new_anonyimized_name, file_extension):
    result_filename = pathlib.Path(wsi_filepath).joinpath(f"{new_anonyimized_name}.{file_extension}")
    if result_filename.exists():
        remove_file(str(result_filename.absolute()))

    wsi_filename = str(pathlib.Path(wsi_filepath).joinpath(f"{original_filename}.{file_extension}").absolute())
    result = anonymize_wsi(wsi_filename, new_anonyimized_name)
    assert result != -1

    assert wait_until_exists(str(result_filename), 5)

    # TODO: add some ordinary checks here?
    
    cleanup(str(result_filename.absolute()))