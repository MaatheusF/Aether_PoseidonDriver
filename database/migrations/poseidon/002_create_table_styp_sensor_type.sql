/*Cria a tabela 'styp_sensor_type' no schema 'poseidon' contendo os possiveis tipos de sensores disponível */
CREATE TABLE IF NOT EXISTS poseidon.styp_sensor_type (
    id SERIAL PRIMARY KEY,
    sensor_name VARCHAR(50),
    sensor_unit VARCHAR(20),
    create_date TIMESTAMPTZ DEFAULT NOW()
);

COMMENT ON TABLE poseidon.styp_sensor_type IS 'Contem os tipos possiveis de sensores disponíveis e seus atributos';
COMMENT ON column poseidon.styp_sensor_type.id IS 'Codigo sequencial da tabela';
COMMENT ON column poseidon.styp_sensor_type.sensor_name IS 'Nome do Sensor';
COMMENT ON column poseidon.styp_sensor_type.sensor_unit IS 'Unidade de medida esperada';
COMMENT ON column poseidon.styp_sensor_type.create_date IS 'Data/Hora em que o tipo foi adicionado';