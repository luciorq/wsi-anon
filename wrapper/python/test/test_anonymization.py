import os
import shutil
import threading
import time
import pytest
import openslide

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
    "wsi_filename, new_label_name, result_label_name",
    [
        ("/data/Aperio/CMU-1.svs", "anon-aperio", "/data/Aperio/anon-aperio.svs"),
        ("/data/Hamamatsu/OS-1.ndpi", "anon-hamamatsu", "/data/Hamamatsu/anon-hamamatsu.ndpi"),
        #("/data/MIRAX/Mirax2.2-1.mrxs", "anon-mirax1", "/data/MIRAX/anon-mirax1.mrxs"), # ToDo: find bug here!
        ("/data/Ventana/OS-2.bif", "anon-ventana1", "/data/Ventana/anon-ventana1.bif"),
    ],
)
def test_anonymize_file_format(cleanup, wsi_filename, new_label_name, result_label_name):
    if os.path.exists(result_label_name):
        remove_file(result_label_name)

    result = anonymize_wsi(wsi_filename, new_label_name)
    assert result in result_label_name

    slide = openslide.OpenSlide(result_label_name)
    assert "label" not in slide.associated_images

    if "Ventana" not in wsi_filename:
        assert "macro" not in slide.associated_images

    if wsi_filename == "/data/Aperio/CMU-1.svs":
        replacement = "XXXXX"
        assert replacement in slide.properties["aperio.Filename"]
        assert replacement in slide.properties["aperio.User"]
        assert replacement in slide.properties["aperio.Date"]
    elif wsi_filename == "/data/Ventana/OS-2.bif":
        replacement = "     "
        assert replacement in slide.properties["ventana.UnitNumber"]
        assert replacement in slide.properties["ventana.UserName"]
        assert replacement in slide.properties["ventana.BuildDate"]
    elif wsi_filename == "/data/MIRAX/Mirax2.2-1.mrxs":
        replacement = "XXXXX"
        assert replacement in slide.properties["mirax.GENERAL.SLIDE_NAME"]
        assert replacement in slide.properties["mirax.GENERAL.PROJECT_NAME"]
        assert replacement in slide.properties["mirax.GENERAL.SLIDE_VERSION"]
        assert replacement in slide.properties["mirax.GENERAL.SLIDE_CREATIONDATETIME"]
        assert "00000" in slide.properties["mirax.GENERAL.SLIDE_ID"]
    else:
        # ToDo: check for Hamamatsu metadata!
        pass

    slide.close()

    cleanup(result_label_name)


@pytest.mark.parametrize(
    "wsi_filename, new_label_name, result_label_name",
    [
        ("/data/Aperio/CMU-1.svs", "anon-aperio", "/data/Aperio/anon-aperio.svs"),
        #("/data/MIRAX/Mirax2.2-1.mrxs", "anon-mirax2", "/data/MIRAX/anon-mirax2.mrxs"), # ToDo: find bug here!
    ],
)
def test_anonymize_file_format_only_label(cleanup, wsi_filename, new_label_name, result_label_name):
    if os.path.exists(result_label_name):
        remove_file(result_label_name)

    result = anonymize_wsi(filename=wsi_filename, new_label_name=new_label_name, keep_macro_image=True, disable_unlinking=False, do_inplace=False)
    assert result in result_label_name

    slide = openslide.OpenSlide(result_label_name)
    assert "label" not in slide.associated_images
    assert "macro" in slide.associated_images
    slide.close()

    cleanup(result_label_name)


@pytest.mark.parametrize(
    "wsi_filename, new_label_name, result_label_name",
    [
        ("/data/Hamamatsu/OS-1.ndpi", "anon-hamamatsu", "/data/Hamamatsu/anon-hamamatsu.ndpi"),
    ],
)
def test_anonymize_file_format_only_label_hamamatsu(wsi_filename, new_label_name, result_label_name):
    if os.path.exists(result_label_name):
        remove_file(result_label_name)

    result = anonymize_wsi(wsi_filename, new_label_name, keep_macro_image=True)
    assert result in result_label_name

    slide = openslide.OpenSlide(result_label_name)
    assert "label" not in slide.associated_images
    assert "macro" not in slide.associated_images
    
    slide.close()
