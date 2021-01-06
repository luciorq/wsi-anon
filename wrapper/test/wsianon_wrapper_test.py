import os
import zipfile
from urllib.request import urlretrieve

import pytest

import wrapper.wsianon as wsianon

def load_example_data(download_folder="/data"):
    if not os.path.exists(os.path.join(download_folder, "Aperio")):
        print("Beginning file download (13 GB)...")
        url = "https://nextcloud.empaia.org/s/4fpdFEn69gqgrgK/download"
        urlretrieve(url, os.path.join(download_folder, "..", "testdata.zip"))
        with zipfile.ZipFile(os.path.join(download_folder, "..", "testdata.zip"), "r") as zip_ref:
            zip_ref.extractall(os.path.join(download_folder, ".."))
        print("Done")

def setup_environment_variables():
    if os.path.exists("/data/OpenSlide_adapted"):
        os.environ["data_dir"] = "/data/OpenSlide_adapted"
    else:
        test_folder = os.path.dirname(os.path.realpath(__file__))
        os.environ["data_dir"] = os.path.join(test_folder, "data", "OpenSlide_adapted")

@pytest.fixture()
def setup():
    setup_environment_variables()
    if not os.path.exists(os.environ["data_dir"]):
        os.mkdir(os.environ["data_dir"])
    load_example_data(os.path.join(os.environ["data_dir"]))

@pytest.mark.parametrize(
    "filename, expected",
    [
        ("CMU-1.svs", wsianon.Vendor.Aperio),
        ("CMU-1.ndpi", wsianon.Vendor.Hamamatsu),
        ("CMU-1.mrxs", wsianon.Vendor.Mirax),
        ("Some-other-file", wsianon.Vendor.Unknown),
    ],
)
def test_check_vendor(filename, expected):
    result = wsianon.check_file_format(filename)
    assert result == expected

@pytest.mark.parametrize(
    "filename, label_name, expected",
    [
        
    ],
)
def test_anonymize_wsi(filename, label_name, expected):
    result = wsianon.anonymize_wsi(filename, label_name)
    assert result == 1