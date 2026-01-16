<?php

namespace App\Http\Controllers\Api;

use App\Http\Controllers\Controller;
use Illuminate\Support\Facades\DB;
use Illuminate\Http\Request;

class GeoController extends Controller
{
    public function latest(string $device)
    {
        $row = DB::table('telemetry')
            ->where('device_id', $device)
            ->orderBy('created_at', 'desc')
            ->first(['device_id','latitude','longitude','temperature','humidity','created_at']);

        if (!$row) {
            return response()->json(['error' => 'Device not found or no telemetry'], 404);
        }

        return response()->json($row);
    }

    public function track(Request $request, string $device)
    {
        $hours = (int)($request->query('hours', 24));
        if ($hours < 1) $hours = 1;
        if ($hours > 168) $hours = 168; // cap at 7 days for sanity

        $rows = DB::table('telemetry')
            ->where('device_id', $device)
            ->where('created_at', '>=', DB::raw("now() - interval '{$hours} hours'"))
            ->orderBy('created_at', 'asc')
            ->get(['latitude','longitude','created_at']);

        return response()->json($rows);
    }
}
