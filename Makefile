.PHONY: help install install-frontend install-backend start stop frontend backend dev clean

# Default target
help:
	@echo "Available targets:"
	@echo "  make install          - Install all dependencies (frontend + backend)"
	@echo "  make install-frontend - Install frontend dependencies"
	@echo "  make install-backend  - Install backend dependencies"
	@echo "  make start            - Start both frontend and backend"
	@echo "  make dev              - Alias for 'make start'"
	@echo "  make frontend         - Start frontend only"
	@echo "  make backend          - Start backend only"
	@echo "  make stop             - Stop all running servers"
	@echo "  make clean            - Stop servers and clean build artifacts"

# Install all dependencies
install: install-frontend install-backend
	@echo "All dependencies installed successfully!"

# Install frontend dependencies
install-frontend:
	@echo "Installing frontend dependencies..."
	cd frontend && npm install

# Install backend dependencies
install-backend:
	@echo "Installing backend dependencies..."
	cd backend && pip install -r requirements.txt

# Start both frontend and backend
start:
	@echo "Starting backend server..."
	@cd backend && python app.py > ../backend.log 2>&1 & echo $$! > ../backend.pid
	@echo "Backend started (PID: $$(cat backend.pid))"
	@echo "Starting frontend server..."
	@cd frontend && npm run dev > ../frontend.log 2>&1 & echo $$! > ../frontend.pid
	@echo "Frontend started (PID: $$(cat frontend.pid))"
	@echo ""
	@echo "=========================================="
	@echo "Servers are running!"
	@echo "Frontend: http://localhost:3000"
	@echo "Backend:  http://localhost:5000"
	@echo "=========================================="
	@echo ""
	@echo "Logs:"
	@echo "  Backend:  tail -f backend.log"
	@echo "  Frontend: tail -f frontend.log"
	@echo ""
	@echo "To stop servers: make stop"

# Alias for start
dev: start

# Start frontend only
frontend:
	@echo "Starting frontend server..."
	cd frontend && npm run dev

# Start backend only
backend:
	@echo "Starting backend server..."
	cd backend && python app.py

# Stop all servers
stop:
	@echo "Stopping servers..."
	@if [ -f backend.pid ]; then \
		kill $$(cat backend.pid) 2>/dev/null || true; \
		rm backend.pid; \
		echo "Backend stopped"; \
	fi
	@if [ -f frontend.pid ]; then \
		kill $$(cat frontend.pid) 2>/dev/null || true; \
		rm frontend.pid; \
		echo "Frontend stopped"; \
	fi
	@echo "All servers stopped"

# Clean up logs and build artifacts
clean: stop
	@echo "Cleaning up..."
	@rm -f backend.log frontend.log
	@echo "Cleanup complete"
