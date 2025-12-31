#!/bin/bash
# HW05 Stress Test Script
# Генерирует нагрузку на сервис для тестирования метрик

set -e

# ============================================
# КОНФИГУРАЦИЯ
# ============================================
BASE_URL="${BASE_URL:-http://arch.homework/otusapp/tobolkin}"
DURATION="${DURATION:-300}"      # Длительность в секундах (по умолчанию 5 минут)
CONCURRENCY="${CONCURRENCY:-50}" # Параллельных соединений

echo "=========================================="
echo "  HW05 Stress Test"
echo "=========================================="
echo "Base URL:    $BASE_URL"
echo "Duration:    ${DURATION}s ($(($DURATION / 60)) мин)"
echo "Concurrency: $CONCURRENCY"
echo "=========================================="
echo ""

# ============================================
# ПРОВЕРКА ЗАВИСИМОСТЕЙ
# ============================================
if ! command -v ab &> /dev/null; then
    echo "Apache Benchmark (ab) не найден. Установка..."
    sudo apt-get update && sudo apt-get install -y apache2-utils
fi

# ============================================
# ПРОВЕРКА СЕРВИСА
# ============================================
echo "Проверка доступности сервиса..."
if ! curl -sf "$BASE_URL/health" > /dev/null 2>&1; then
    echo "ERROR: Сервис недоступен по адресу $BASE_URL/health"
    echo ""
    echo "Проверьте:"
    echo "  1. minikube tunnel запущен в отдельном терминале"
    echo "  2. /etc/hosts содержит правильный IP для arch.homework"
    echo "     Команда: kubectl get svc -n m nginx-ingress-nginx-controller"
    exit 1
fi
echo "✓ Сервис доступен"
echo ""

# ============================================
# СОЗДАНИЕ ТЕСТОВЫХ ДАННЫХ
# ============================================
TIMESTAMP=$(date +%s)
cat > /tmp/post_user.json << EOF
{"username":"stress_${TIMESTAMP}","firstName":"Stress","lastName":"Test","email":"stress_${TIMESTAMP}@test.com","phone":"+1-555-0000"}
EOF

echo '{"invalid json' > /tmp/bad_json.json

# ============================================
# ЗАПУСК НАГРУЗКИ
# ============================================
echo "Запуск нагрузки на $DURATION секунд..."
echo "Нажмите Ctrl+C для остановки"
echo ""

START_TIME=$(date +%s)
END_TIME=$((START_TIME + DURATION))
ITERATION=0

while [ $(date +%s) -lt $END_TIME ]; do
    ITERATION=$((ITERATION + 1))
    REMAINING=$((END_TIME - $(date +%s)))
    
    echo "--- Итерация $ITERATION (осталось ${REMAINING}s) ---"
    
    # GET /api/v1/users - основная нагрузка
    echo -n "GET /users: "
    ab -n 5000 -c $CONCURRENCY -q "$BASE_URL/api/v1/users" 2>/dev/null | grep "Requests per second" || echo "done"
    
    # GET /api/v1/users/{id}
    echo -n "GET /users/1: "
    ab -n 5000 -c $CONCURRENCY -q "$BASE_URL/api/v1/users/1" 2>/dev/null | grep "Requests per second" || echo "done"
    
    # POST с ошибками (unique constraint) - генерирует 4xx/5xx
    echo -n "POST (errors): "
    ab -n 500 -c 10 -p /tmp/post_user.json -T "application/json" -q "$BASE_URL/api/v1/users" 2>/dev/null | grep "Non-2xx" || echo "done"
    
    # Некорректный JSON - генерирует ошибки
    echo -n "Bad JSON: "
    ab -n 200 -c 10 -p /tmp/bad_json.json -T "application/json" -q "$BASE_URL/api/v1/users" 2>/dev/null | grep "Non-2xx" || echo "done"
    
    # 404 ошибки
    echo -n "404 errors: "
    ab -n 500 -c 10 -q "$BASE_URL/api/v1/users/999999" 2>/dev/null | grep "Non-2xx" || echo "done"
    
    echo ""
done

# ============================================
# ИТОГИ
# ============================================
ACTUAL_DURATION=$(($(date +%s) - START_TIME))
echo "=========================================="
echo "  ✅ Stress Test завершён!"
echo "=========================================="
echo "Выполнено итераций: $ITERATION"
echo "Фактическое время:  ${ACTUAL_DURATION}s"
echo ""
echo "Проверьте дашборд Grafana:"
echo "  http://localhost:3000"
echo ""

# Очистка
rm -f /tmp/post_user.json /tmp/bad_json.json
