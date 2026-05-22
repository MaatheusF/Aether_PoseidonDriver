/*Cria a tabela 'rely_relay' no schema 'poseidon' contendo os relay disponível*/
CREATE TABLE IF NOT EXISTS poseidon.rely_relay (
	id SERIAL PRIMARY KEY,
	device_id INTEGER NOT NULL REFERENCES poseidon.devc_device (id) ON DELETE CASCADE,
	external_id VARCHAR(100) NOT NULL,
	relay_name VARCHAR(100) NOT NULL,
	active BOOLEAN DEFAULT TRUE,
	create_date TIMESTAMP DEFAULT NOW(),
	UNIQUE (device_id, external_id)
);

COMMENT ON TABLE poseidon.rely_relay IS 'Contem o cadastro de relay associado a um dispositivo';
COMMENT ON column poseidon.rely_relay.id IS 'Codigo sequencial da tabela';
COMMENT ON column poseidon.rely_relay.device_id IS 'Codigo do dispositivo';
COMMENT ON column poseidon.rely_relay.external_id IS 'Codigo externo do relay';
COMMENT ON column poseidon.rely_relay.relay_name IS 'Nome do relay';
COMMENT ON column poseidon.rely_relay.active IS 'Status do relay';
COMMENT ON column poseidon.rely_relay.create_date IS 'Data/Hora em que o dado foi cadatrado';