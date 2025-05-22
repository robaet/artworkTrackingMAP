package com.example.locationlogger

import android.Manifest
import android.content.pm.PackageManager
import android.location.Location
import android.os.Bundle
import android.os.Looper
import android.widget.Button
import android.widget.TextView
import android.widget.Toast
import androidx.activity.result.contract.ActivityResultContracts
import androidx.appcompat.app.AppCompatActivity
import androidx.core.content.ContextCompat
import androidx.core.app.ActivityCompat
import com.google.android.gms.location.*
import java.io.File
import java.io.IOException

class MainActivity : AppCompatActivity() {

    private lateinit var locationTextView: TextView
    private lateinit var clearButton: Button
    private lateinit var fusedLocationClient: FusedLocationProviderClient
    private lateinit var locationCallback: LocationCallback
    private val locationRequest: LocationRequest = LocationRequest.Builder(Priority.PRIORITY_HIGH_ACCURACY, 4000) // Interval is in milliseconds
        .setMinUpdateIntervalMillis(4000) // 4 seconds
        .setPriority(Priority.PRIORITY_HIGH_ACCURACY)
        .build()

    private val requestPermissionLauncher =
        registerForActivityResult(ActivityResultContracts.RequestPermission()) { isGranted ->
            if (isGranted) {
                startLocationUpdates()
            } else {
                Toast.makeText(this, "Location permission is required to log location data", Toast.LENGTH_SHORT).show()
            }
        }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        locationTextView = findViewById(R.id.locationTextView)

        clearButton = findViewById(R.id.clearButton)

        fusedLocationClient = LocationServices.getFusedLocationProviderClient(this)
        setupLocationCallback()

        // Check and request location permission
        checkLocationPermissionAndStartUpdates()

        clearButton.setOnClickListener {
            clearLogFile()
        }
    }

    private fun checkLocationPermissionAndStartUpdates() {
        when {
            ContextCompat.checkSelfPermission(this, Manifest.permission.ACCESS_FINE_LOCATION) == PackageManager.PERMISSION_GRANTED -> {
                // Permission is already granted; start location updates
                startLocationUpdates()
            }
            ActivityCompat.shouldShowRequestPermissionRationale(this, Manifest.permission.ACCESS_FINE_LOCATION) -> {
                // Show an explanation to the user and request permission again
                Toast.makeText(this, "Location permission is needed to log location data", Toast.LENGTH_LONG).show()
                requestPermissionLauncher.launch(Manifest.permission.ACCESS_FINE_LOCATION)
            }
            else -> {
                // Directly request permission if it hasn’t been requested before
                requestPermissionLauncher.launch(Manifest.permission.ACCESS_FINE_LOCATION)
            }
        }
    }

    private fun setupLocationCallback() {
        locationCallback = object : LocationCallback() {
            override fun onLocationResult(locationResult: LocationResult) {
                locationResult.locations.forEach { location ->
                    logLocation(location)
                }
            }
        }
    }

    private fun startLocationUpdates() {
        if (ContextCompat.checkSelfPermission(this, Manifest.permission.ACCESS_FINE_LOCATION) == PackageManager.PERMISSION_GRANTED) {
            fusedLocationClient.requestLocationUpdates(locationRequest, locationCallback, Looper.getMainLooper())
        }
    }

    private fun logLocation(location: Location) {
        val timestamp = System.currentTimeMillis() / 1000
        val logEntry = "\n$timestamp, ${location.latitude}, ${location.longitude}\n"
        locationTextView.append(logEntry)
        writeToFile(logEntry)
    }

    private fun writeToFile(data: String) {
        try {
            val file = File(getExternalFilesDir(null), "location_log.txt")
            file.appendText(data)
        } catch (e: IOException) {
            e.printStackTrace()
        }
    }

    private fun clearLogFile() {
        try {
            val file = File(getExternalFilesDir(null), "location_log.txt")
            if (file.exists()) {
                // Clear the file by writing an empty string
                file.writeText("")  // Overwrite with empty content
                locationTextView.text = ""  // Clear the TextView as well
                Toast.makeText(this, "Location log cleared", Toast.LENGTH_SHORT).show()
            } else {
                Toast.makeText(this, "No log file found", Toast.LENGTH_SHORT).show()
            }
        } catch (e: IOException) {
            e.printStackTrace()
            Toast.makeText(this, "Failed to clear the log file", Toast.LENGTH_SHORT).show()
        }
    }

    override fun onStop() {
        super.onStop()
        fusedLocationClient.removeLocationUpdates(locationCallback)
    }
}