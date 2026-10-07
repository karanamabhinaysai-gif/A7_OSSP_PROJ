@echo off
title OSSP Interactive Cloud Terminal
echo ===================================================
echo   OSSP Interactive Cloud Terminal Studio
echo ===================================================
echo Starting backend server on http://localhost:5050...
start "" http://localhost:5050
python server.py
pause
