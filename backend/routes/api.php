<?php

use Illuminate\Support\Facades\Route;
use App\Http\Controllers\Api\DeviceController;
use App\Http\Controllers\Api\GeoController;

Route::get('/probe', function () {
    return response()->json(['ok' => true, 'where' => 'routes/api.php']);
});

Route::get('/devices', [DeviceController::class, 'index']);
Route::get('/devices/{device}/history', [DeviceController::class, 'history']);

Route::get('/devices/{device}/latest', [GeoController::class, 'latest']);
Route::get('/devices/{device}/track',  [GeoController::class, 'track']);
