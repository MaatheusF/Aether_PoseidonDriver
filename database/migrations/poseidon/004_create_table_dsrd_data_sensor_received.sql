/*Cria a tabela 'dsrd_data_sensor_received' no schema 'poseidon' contendo os dados recebidos dos sensores */
CREATE TABLE IF NOT EXISTS poseidon.dsrd_data_sensor_received (
    id SERIAL PRIMARY KEY,
    sensor_id INTEGER NOT NULL REFERENCES poseidon.sens_sensor (id),
    device_id INTEGER NOT NULL REFERENCES poseidon.devc_device (id),
    data_value DECIMAL,
    read_date TIMESTAMPTZ,
    create_date TIMESTAMPTZ DEFAULT NOW()
);

COMMENT ON TABLE poseidon.dsrd_data_sensor_received IS 'Contem os dados recebidos dos sensores instalados';
COMMENT ON column poseidon.dsrd_data_sensor_received.id IS 'Codigo sequencial da tabela';
COMMENT ON column poseidon.dsrd_data_sensor_received.sensor_id IS 'Codigo do sensor';
COMMENT ON column poseidon.dsrd_data_sensor_received.device_id IS 'Codigo do dispositivo';
COMMENT ON column poseidon.dsrd_data_sensor_received.data_value IS 'Valor do sensor recebido';
COMMENT ON column poseidon.dsrd_data_sensor_received.read_date IS 'Data/Hora em que o dado foi gerado no sensor';
COMMENT ON column poseidon.dsrd_data_sensor_received.create_date IS 'Data/Hora em que o dado foi cadatrado';
