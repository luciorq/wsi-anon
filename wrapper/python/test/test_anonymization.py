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
        #("/data/MIRAX/Mirax2.2-1.mrxs", "anon-mirax1", "/data/MIRAX/anon-mirax1.mrxs"), # TODO: OpenSlide occasionally throws error while initializing
        #("/data/Ventana/OS-2.bif", "anon-ventana1", "/data/Ventana/anon-ventana1.bif"), # TODO: might be related to above issue
    ],
)
def test_anonymize_file_format(cleanup, wsi_filename, new_label_name, result_label_name):
    if os.path.exists(result_label_name):
        remove_file(result_label_name)

    result = anonymize_wsi(wsi_filename, new_label_name)
    assert result in result_label_name

    time.sleep(1)

    with openslide.OpenSlide(result_label_name) as slide:
        assert "label" not in slide.associated_images

        if "Ventana" not in wsi_filename:
            assert "macro" not in slide.associated_images

        if "Aperio" in wsi_filename:
            for property in ["Filename", "User", "Date"]:
                assert all(c == "X" for c in slide.properties[f"aperio.{property}"])
        if "Ventana" in wsi_filename:
            for property in ["UnitNumber", "UserName", "BuildDate"]:
                assert all(c == " " for c in slide.properties[f"ventana.{property}"])
        if "MIRAX" in wsi_filename:
            for property in ["SLIDE_NAME", "PROJECT_NAME", "SLIDE_CREATIONDATETIME"]:
                assert all(c == "X" for c in slide.properties[f"mirax.GENERAL.{property}"])
            assert all(c == "0" for c in slide.properties[f"mirax.GENERAL.SLIDE_ID"])
        if "Hamamatsu" in wsi_filename:
            # ToDo: check for Hamamatsu metadata!
            pass

    cleanup(result_label_name)


@pytest.mark.parametrize(
    "wsi_filename, new_label_name, result_label_name",
    [
        ("/data/Aperio/CMU-1.svs", "anon-aperio", "/data/Aperio/anon-aperio.svs"),
        #("/data/MIRAX/Mirax2.2-1.mrxs", "anon-mirax2", "/data/MIRAX/anon-mirax2.mrxs"), # TODO: OpenSlide occasionally throws error while initializing
    ],
)
def test_anonymize_file_format_only_label(cleanup, wsi_filename, new_label_name, result_label_name):
    if os.path.exists(result_label_name):
        remove_file(result_label_name)

    result = anonymize_wsi(filename=wsi_filename, new_label_name=new_label_name, keep_macro_image=True, disable_unlinking=False, do_inplace=False)
    assert result in result_label_name

    time.sleep(1)

    with openslide.OpenSlide(result_label_name) as slide:
        assert "label" not in slide.associated_images
        assert "macro" in slide.associated_images

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

    time.sleep(1)

    with openslide.OpenSlide(result_label_name) as slide:
        assert "label" not in slide.associated_images
        assert "macro" not in slide.associated_images
