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
    # Check that LED controls are present
    assert "LED" in response.text

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
    assert 'led_state' in data  # Updated to expect led_state in response
    assert 'chip_model' in data
    assert 'cores' in data

def test_led_control_endpoints(dut):
    """Test that the LED control endpoints work properly"""
    # Get the IP address from the DUT (Device Under Test)
    dut.expect('Starting HTTP Server')
    ip_address = dut.expect(r'Access the web server at http://(\d+\.\d+\.\d+\.\d+)')[0]
    
    # Test LED on endpoint
    url = f"http://{ip_address}/ledon"
    response = requests.get(url)
    assert response.status_code == 200
    assert "включен" in response.text or "on" in response.text
    
    # Test LED off endpoint
    url = f"http://{ip_address}/ledoff"
    response = requests.get(url)
    assert response.status_code == 200
    assert "выключен" in response.text or "off" in response.text
    
    # Test LED toggle endpoint
    url = f"http://{ip_address}/ledtoggle"
    response = requests.get(url)
    assert response.status_code == 200
    assert "переключен" in response.text or "toggled" in response.text