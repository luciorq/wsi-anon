import os
import shutil
import pytest

from ..wsianon import check_file_format, anonymize_wsi, Vendor


@pytest.fixture(scope='session', autouse=True)
def cleanup():
    temporary_files = []
    def add_filename(filename):
        temporary_files.append(filename)

    yield add_filename

    for filename in temporary_files:
        if filename.endswith(".mrxs"):
            mrxs_path = filename[:len(filename)-5]
            shutil.rmtree(mrxs_path)
        os.remove(filename)

@pytest.mark.parametrize(
    "wsi_filename, vendor",
    [
        ("/data/Aperio/CMU-1.svs", Vendor.Aperio),
        ("/data/Hamamatsu/OS-1.ndpi", Vendor.Hamamatsu),
        ("/data/MIRAX/Mirax2.2-1.mrxs", Vendor.Mirax),
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
        #("/data/MIRAX/Mirax2.2-1.mrxs", "anon-mirax", "/data/MIRAX/anon-mirax"),
    ],
)
def test_anonymize_file_format(cleanup, wsi_filename, new_label_name, result_label_name):
    result = anonymize_wsi(wsi_filename, new_label_name)
    assert result == result_label_name

    cleanup(result_label_name)