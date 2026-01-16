<?php

namespace App\Http\Controllers\Api;

use App\Http\Controllers\Controller;
use Illuminate\Support\Facades\DB;

class DeviceController extends Controller
{
    public function index()
    {
        return DB::table('telemetry')
            ->select('device_id')
            ->distinct()
            ->get();
    }

    public function history($device)
    {
        return DB::table('telemetry')
            ->where('device_id', $device)
            ->orderBy('created_at', 'desc')
            ->limit(1000)
            ->get(['latitude', 'longitude', 'created_at']);
    }
}

