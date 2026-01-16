<?php

use Illuminate\Support\Facades\Route;

Route::get('/', function () {
    return view('hub');
});

Route::get('/globe', function () {
    return view('globe');
});

