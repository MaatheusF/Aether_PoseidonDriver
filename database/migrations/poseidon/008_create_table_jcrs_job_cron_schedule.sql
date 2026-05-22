/*Cria a tabela 'jcrs_job_cron_schedule' no schema 'poseidon' contendo os dados das rotinas*/
CREATE TABLE IF NOT EXISTS poseidon.jcrs_job_cron_schedule (
	id SERIAL PRIMARY KEY,
	device_id INTEGER REFERENCES poseidon.devc_device (id),
	relay_id INTEGER REFERENCES poseidon.rely_relay (id),
	job_type VARCHAR(50) NOT NULL, -- RELAY_ON | RELAY_OFF | HEARTBEAT_CHECK
	payload JSONB, -- dados adicionais
	job_name VARCHAR(100),
	cron_expression VARCHAR(50),
	cron_schedule_datetime TIMESTAMP,
	last_run_datetime TIMESTAMP,
	active BOOLEAN DEFAULT TRUE,
	create_date TIMESTAMP DEFAULT NOW()
);

COMMENT ON TABLE poseidon.jcrs_job_cron_schedule IS 'Contem o agendamento de tarefas do modulo';
COMMENT ON column poseidon.jcrs_job_cron_schedule.id IS 'Codigo sequencial da tabela';
COMMENT ON column poseidon.jcrs_job_cron_schedule.device_id IS 'Codigo do dispositivo de destino (Caso seja enviado um comando via TCP)';
COMMENT ON column poseidon.jcrs_job_cron_schedule.relay_id IS 'Codigo do relay de destino (Caso seja enviado um comando via TCP)';
COMMENT ON column poseidon.jcrs_job_cron_schedule.job_type IS 'Tipo do agendamento: RELAY_ON | RELAY_OFF | HEARTBEAT_CHECK';
COMMENT ON column poseidon.jcrs_job_cron_schedule.payload IS 'Dados adicionais no formato JSON';
COMMENT ON column poseidon.jcrs_job_cron_schedule.job_name IS 'Nome de identificação do agendamento';
COMMENT ON column poseidon.jcrs_job_cron_schedule.cron_expression IS 'Definição do periodo de execução do agendamento no formato de crontab * * *';
COMMENT ON column poseidon.jcrs_job_cron_schedule.cron_schedule_datetime IS 'Data/Hora em que será executado (Caso cron_expression esteja vazio)';
COMMENT ON column poseidon.jcrs_job_cron_schedule.last_run_datetime IS 'Data/Hora da ultima execução';
COMMENT ON column poseidon.jcrs_job_cron_schedule.active IS 'Status do agendamento';
COMMENT ON column poseidon.jcrs_job_cron_schedule.create_date IS 'Data/Hora em que o dado foi cadatrado';