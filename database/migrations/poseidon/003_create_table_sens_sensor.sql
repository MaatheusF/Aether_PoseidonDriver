/*Cria a tabela 'sens_sensor' no schema 'poseidon' contendo os sensores instalados */
CREATE TABLE IF NOT EXISTS poseidon.sens_sensor (
    id SERIAL PRIMARY KEY,
    device_id INTEGER NOT NULL REFERENCES poseidon.devc_device (id),
    sensor_type_id INTEGER NOT NULL REFERENCES poseidon.styp_sensor_type (id),
    create_date TIMESTAMPTZ DEFAULT NOW(),
    active BOOLEAN
);

COMMENT ON TABLE poseidon.sens_sensor IS 'Contem os sensores instalados';
COMMENT ON column poseidon.sens_sensor.id IS 'Codigo sequencial da tabela';
COMMENT ON column poseidon.sens_sensor.device_id IS 'Codigo do dispositivo';
COMMENT ON column poseidon.sens_sensor.sensor_type_id IS 'Codigo do tipo do sensor';
COMMENT ON column poseidon.sens_sensor.create_date IS 'Data/Hora em que o sensor foi adicionado';
COMMENT ON column poseidon.sens_sensor.active IS 'Status definindo se o sensor está ativo ou inativo';