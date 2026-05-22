/*Cria a tabela 'devc_device' no schema 'poseidon' contendo a lista dispositivos (ESP32, Arduino) */
CREATE TABLE IF NOT EXISTS poseidon.devc_device (
    id SERIAL PRIMARY KEY,
    module_id INTEGER NOT NULL REFERENCES aether_core.amod_aether_module (id),
    device_name VARCHAR(50),
    device_type VARCHAR(50),
    create_date TIMESTAMPTZ DEFAULT NOW(),
    active BOOLEAN
);

COMMENT ON TABLE poseidon.devc_device IS 'Contem a lista dispositivos anexados ao modulo';
COMMENT ON column poseidon.devc_device.id IS 'Codigo sequencial da tabela';
COMMENT ON column poseidon.devc_device.module_id IS 'Codigo do modulo (No schema aether_core)';
COMMENT ON column poseidon.devc_device.device_name IS 'Nome do dispositivo';
COMMENT ON column poseidon.devc_device.device_type IS 'Tipo do dispositivo';
COMMENT ON column poseidon.devc_device.create_date IS 'Data/Hora em que o dispositivo foi adicionado';
COMMENT ON column poseidon.devc_device.active IS 'Status definindo se o dispositivo está ativo ou inativo';