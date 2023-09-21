# Edit following two lines to set component requirements (see docs)
idf_component_register(SRCS main.c wifi_app.c http_server.c adc.c app_nvs.c sntp_time_sync.c mqtt.c fft.c cJSON.c
						INCLUDE_DIRS "."
						EMBED_FILES webpage/app.css webpage/app.js webpage/favicon.ico webpage/index.html webpage/jquery-3.3.1.min.js webpage/chart.js)
						