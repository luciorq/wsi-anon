import os
import pathlib
import shutil
import threading
import time
import pytest
import openslide
import tiffslide

from ..wsianon import check_file_format, anonymize_wsi, Vendor


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
            break
        time.sleep(1)
        max_wait_in_sec -= 1


@pytest.mark.parametrize(
    "wsi_filename, vendor",
    [
        ("/data/Aperio/CMU-1.svs", Vendor.Aperio),
        ("/data/Hamamatsu/OS-1.ndpi", Vendor.Hamamatsu),
        ("/data/MIRAX/Mirax2.2-1.mrxs", Vendor.Mirax),
        ("/data/Ventana/OS-2.bif", Vendor.Ventana),
    ],
)
def test_check_fileformat(wsi_filename, vendor):
    file_format = check_file_format(wsi_filename)
    assert file_format == vendor


@pytest.mark.parametrize(
    "wsi_filepath, original_filename, new_anonyimized_name, file_extension",
    [
        ("/data/Aperio/", "CMU-1", "anon-aperio", "svs"),
        ("/data/Hamamatsu/", "OS-1", "anon-hamamatsu", "ndpi"),
    ],
)
def test_anonymize_file_format_tiffslide(cleanup, wsi_filepath, original_filename, new_anonyimized_name, file_extension):
    result_filename = pathlib.Path(wsi_filepath).joinpath(f"{new_anonyimized_name}.{file_extension}")
    if result_filename.exists():
        remove_file(str(result_filename.absolute()))

    wsi_filename = str(pathlib.Path(wsi_filepath).joinpath(f"{original_filename}.{file_extension}").absolute())
    result = anonymize_wsi(wsi_filename, new_anonyimized_name)
    assert result != -1
    
    wait_until_exists(str(result_filename), 5)

    try:
        slide = tiffslide.TiffSlide(result_filename)
        associated_images = slide.associated_images
        assert "label" not in associated_images
        
        if "Aperio" in wsi_filepath:
            for property in ["Filename", "User", "Date"]:
                assert all(c == "X" for c in slide.properties[f"aperio.{property}"])
        if "Hamamatsu" in wsi_filepath:
            # ToDo: check for Hamamatsu metadata!
            pass
        
        slide.close()
    except tiffslide.TiffFileError as e:
        assert False
    
    cleanup(str(result_filename.absolute()))


@pytest.mark.parametrize(
    "wsi_filepath, original_filename, new_anonyimized_name, file_extension",
    [
        ("/data/Ventana/", "OS-2", "anon-ventana", "bif"),
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
    
    wait_until_exists(str(result_filename), 5)

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
    
    wait_until_exists(str(result_filename), 5)

    try:
        slide = tiffslide.TiffSlide(result_filename)
        associated_images = slide.associated_images
        assert "label" not in associated_images
        assert "macro" in associated_images
        
        slide.close()
    except tiffslide.TiffFileError as e:
        assert False

    cleanup(str(result_filename.absolute()))


@pytest.mark.parametrize(
    "wsi_filepath, original_filename, new_anonyimized_name, file_extension",
    [
        ("/data/Hamamatsu/", "OS-1", "anon-hamamatsu2", "ndpi"),
    ],
)
def test_anonymize_file_format_only_label_hamamatsu(wsi_filepath, original_filename, new_anonyimized_name, file_extension):
    result_filename = pathlib.Path(wsi_filepath).joinpath(f"{new_anonyimized_name}.{file_extension}")
    if result_filename.exists():
        remove_file(str(result_filename.absolute()))

    wsi_filename = str(pathlib.Path(wsi_filepath).joinpath(f"{original_filename}.{file_extension}").absolute())
    result = anonymize_wsi(wsi_filename, new_anonyimized_name, keep_macro_image=True)
    assert result != -1

    wait_until_exists(str(result_filename), 5)

    with openslide.OpenSlide(str(result_filename)) as slide:
        assert "label" not in slide.associated_images
        assert "macro" not in slide.associated_images
