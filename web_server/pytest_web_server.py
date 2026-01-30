# Pytest test cases for the web server example

import pytest
import requests
import json

def test_http_server_available(dut):
    """Test that the HTTP server is available and responding"""
    # Get the IP address from the DUT (Device Under Test)
    dut.expect('Starting HTTP Server')
    ip_address = dut.expect(r'Access the web server at http://(\d+\.\d+\.\d+\.\d+)')[0]
    
    # Test the root endpoint
    url = f"http://{ip_address}"
    response = requests.get(url)
    assert response.status_code == 200
    assert "ESP32 Web Server" in response.text

def test_status_endpoint(dut):
    """Test that the status endpoint returns JSON data"""
    # Get the IP address from the DUT (Device Under Test)
    dut.expect('Starting HTTP Server')
    ip_address = dut.expect(r'Access the web server at http://(\d+\.\d+\.\d+\.\d+)')[0]
    
    # Test the status endpoint
    url = f"http://{ip_address}/status"
    response = requests.get(url)
    assert response.status_code == 200
    
    # Verify that the response is valid JSON
    data = response.json()
    assert 'uptime' in data
    assert 'free_heap' in data
    assert 'min_free_heap' in data
    assert 'cpu_freq_mhz' in data

def test_info_endpoint(dut):
    """Test that the info endpoint returns device information"""
    # Get the IP address from the DUT (Device Under Test)
    dut.expect('Starting HTTP Server')
    ip_address = dut.expect(r'Access the web server at http://(\d+\.\d+\.\d+\.\d+)')[0]
    
    # Test the info endpoint
    url = f"http://{ip_address}/info"
    response = requests.get(url)
    assert response.status_code == 200
    
    # Verify that the response is valid JSON
    data = response.json()
    assert 'model' in data
    assert 'cores' in data
    assert 'features' in data
    assert 'revision' in data
    assert 'flash_size_mb' in data