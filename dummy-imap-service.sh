#!/bin/bash

cat <<'EOF' > /etc/systemd/system/dummy-imap.service
[Unit]
Description=dummy-imap service
After=network.target
StartLimitIntervalSec=0

[Service]
Type=simple
Restart=always
RestartSec=1
User=nobody
ExecStart=/app/dummy-imap/dummy-imap

[Install]
WantedBy=multi-user.target

EOF

systemctl daemon-reload
systemctl enable dummy-imap

systemctl restart dummy-imap



