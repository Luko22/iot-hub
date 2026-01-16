protected $table = 'telemetry';

protected $fillable = [
    'device_id',
    'latitude',
    'longitude',
    'temperature',
    'humidity',
    'created_at'
];

public $timestamps = false;

