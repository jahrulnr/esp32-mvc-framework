// ESP32 MVC Framework - Main JavaScript
// 
// API Endpoints (all under /api/v1):
// - GET  /api/v1/auth/user          - Get current user info
// - GET  /api/v1/status             - System status
// - GET  /api/v1/health             - Health check
// - GET  /api/v1/version            - Version info
// - GET  /api/v1/users              - List all users (CSV demo)
// - GET  /api/v1/items              - List items
// - POST /api/v1/items              - Create item
// - GET  /api/v1/items/{id}         - Get specific item
// - PUT  /api/v1/items/{id}         - Update item
// - DEL  /api/v1/items/{id}         - Delete item
// - GET  /api/v1/admin/users        - List users (admin)
// - POST /api/v1/admin/users        - Create user (admin)
// - GET  /api/v1/admin/users/{id}   - Get user (admin)
// - PUT  /api/v1/admin/users/{id}   - Update user (admin)
// - DEL  /api/v1/admin/users/{id}   - Delete user (admin)
// - GET  /api/v1/dashboard          - Dashboard data
// - POST /api/v1/restart            - Restart system

class ESP32MVC {
    constructor() {
        this.apiBase = '/api/v1';
        this.token = localStorage.getItem('auth_token');
        this.init();
    }
    
    init() {
        this.setupGlobalErrorHandling();
        this.setupTokenInterceptor();
        this.checkAuthentication();
    }
    
    setupGlobalErrorHandling() {
        window.addEventListener('error', (event) => {
            console.error('Global error:', event.error);
        });
    }
    
    setupTokenInterceptor() {
        // Store original fetch
        const originalFetch = window.fetch;
        const self = this; // Store reference to this
        
        // Override fetch to automatically add auth token
        window.fetch = (...args) => {
            let [url, config] = args;
            
            // Only add token for our API calls
            if (url.startsWith('/api') || url.startsWith('/dashboard')) {
                const token = localStorage.getItem('auth_token');
                if (token) {
                    config = config || {};
                    config.headers = config.headers || {};
                    config.headers['Authorization'] = `Bearer ${token}`;
                }
            }
            
            return originalFetch(url, config)
                .then(response => {
                    // Handle unauthorized responses
                    if (response.status === 401 && !url.includes('/login')) {
                        self.logout(); // Use stored reference
                    }
                    return response;
                });
        };
    }
    
    checkAuthentication() {
        const token = localStorage.getItem('auth_token');
        const currentPath = window.location.pathname;
        
        // Protected routes
        const protectedRoutes = ['/dashboard'];
        const authRoutes = ['/login', '/register'];
        
        if (protectedRoutes.includes(currentPath) && !token) {
            window.location.href = '/login';
        } else if (authRoutes.includes(currentPath) && token) {
            // Verify token is still valid
            this.verifyToken().then(valid => {
                if (valid) {
                    window.location.href = '/dashboard';
                }
            });
        }
    }
    
    async verifyToken() {
        try {
            const token = localStorage.getItem('auth_token');
            if (!token) {
                return false;
            }
            
            const response = await fetch('/api/v1/auth/user', {
                headers: {
                    'Authorization': `Bearer ${token}`
                }
            });
            return response.ok;
        } catch (error) {
            return false;
        }
    }
    
    async apiCall(endpoint, options = {}) {
        const url = endpoint.startsWith('/') ? endpoint : `${this.apiBase}/${endpoint}`;
        
        const defaultOptions = {
            headers: {
                'Content-Type': 'application/json',
            }
        };
        
        // Don't override Content-Type if it's already set (e.g., for form data)
        const mergedOptions = { ...defaultOptions, ...options };
        if (options.headers && options.headers['Content-Type']) {
            mergedOptions.headers = { ...defaultOptions.headers, ...options.headers };
        }
        
        try {
            const response = await fetch(url, mergedOptions);
            
            // Handle different response types
            let data;
            const contentType = response.headers.get('content-type');
            if (contentType && contentType.includes('application/json')) {
                data = await response.json();
            } else {
                data = await response.text();
            }
            
            if (!response.ok) {
                throw new Error(data.message || data || 'API call failed');
            }
            
            return data;
        } catch (error) {
            console.error(`API call failed for ${url}:`, error);
            throw error;
        }
    }
    
    async login(email, password) {
        try {
            const response = await fetch('/login', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/x-www-form-urlencoded',
                },
                body: new URLSearchParams({
                    email: email,
                    password: password
                })
            });
            
            const data = await response.json();
            
            if (data.success) {
                localStorage.setItem('auth_token', data.token);
                localStorage.setItem('user_email', data.user.email);
                localStorage.setItem('user_name', data.user.name);
                return data;
            } else {
                throw new Error(data.message);
            }
        } catch (error) {
            console.error('Login failed:', error);
            throw error;
        }
    }
    
    logout() {
        localStorage.clear();
        window.location.href = '/login';
    }
    
    showAlert(message, type = 'info', duration = 5000) {
        // Remove existing alerts
        const existingAlerts = document.querySelectorAll('.esp32-alert');
        existingAlerts.forEach(alert => alert.remove());
        
        // Create new alert
        const alert = document.createElement('div');
        alert.className = `esp32-alert alert alert-${type}`;
        alert.textContent = message;
        alert.style.cssText = `
            position: fixed;
            top: 20px;
            right: 20px;
            z-index: 9999;
            max-width: 400px;
            opacity: 0;
            transform: translateX(100%);
            transition: all 0.3s ease;
        `;
        
        document.body.appendChild(alert);
        
        // Animate in
        setTimeout(() => {
            alert.style.opacity = '1';
            alert.style.transform = 'translateX(0)';
        }, 100);
        
        // Auto remove
        if (duration > 0) {
            setTimeout(() => {
                alert.style.opacity = '0';
                alert.style.transform = 'translateX(100%)';
                setTimeout(() => alert.remove(), 300);
            }, duration);
        }
    }
    
    formatBytes(bytes) {
        if (bytes === 0) return '0 Bytes';
        const k = 1024;
        const sizes = ['Bytes', 'KB', 'MB', 'GB'];
        const i = Math.floor(Math.log(bytes) / Math.log(k));
        return parseFloat((bytes / Math.pow(k, i)).toFixed(2)) + ' ' + sizes[i];
    }
    
    formatUptime(milliseconds) {
        const seconds = Math.floor(milliseconds / 1000);
        const days = Math.floor(seconds / 86400);
        const hours = Math.floor((seconds % 86400) / 3600);
        const minutes = Math.floor((seconds % 3600) / 60);
        const secs = seconds % 60;
        
        if (days > 0) {
            return `${days}d ${hours}h ${minutes}m`;
        } else if (hours > 0) {
            return `${hours}h ${minutes}m ${secs}s`;
        } else if (minutes > 0) {
            return `${minutes}m ${secs}s`;
        } else {
            return `${secs}s`;
        }
    }
    
    debounce(func, wait) {
        let timeout;
        return function executedFunction(...args) {
            const later = () => {
                clearTimeout(timeout);
                func(...args);
            };
            clearTimeout(timeout);
            timeout = setTimeout(later, wait);
        };
    }
    
    // Form helpers
    setupForm(formId, onSubmit) {
        const form = document.getElementById(formId);
        if (!form) return;
        
        form.addEventListener('submit', async (e) => {
            e.preventDefault();
            const formData = new FormData(form);
            const data = Object.fromEntries(formData.entries());
            
            try {
                await onSubmit(data);
            } catch (error) {
                this.showAlert(error.message, 'error');
            }
        });
    }
    
    // Auto-update functionality
    startAutoUpdate(callback, interval = 5000) {
        const update = () => {
            if (!document.hidden) {
                callback();
            }
        };
        
        // Initial call
        update();
        
        // Set up interval
        const intervalId = setInterval(update, interval);
        
        // Pause when tab is hidden
        document.addEventListener('visibilitychange', () => {
            if (document.hidden) {
                clearInterval(intervalId);
            } else {
                setTimeout(() => {
                    this.startAutoUpdate(callback, interval);
                }, 1000);
            }
        });
        
        return intervalId;
    }
    
    // UI utility methods
    showModal(modalId) {
        const modal = document.getElementById(modalId);
        if (modal) {
            modal.style.display = 'block';
        }
    }
    
    hideModal(modalId) {
        const modal = document.getElementById(modalId);
        if (modal) {
            modal.style.display = 'none';
        }
    }
    
    toggleElement(elementId, show = null) {
        const element = document.getElementById(elementId);
        if (element) {
            if (show === null) {
                element.style.display = element.style.display === 'none' ? 'block' : 'none';
            } else {
                element.style.display = show ? 'block' : 'none';
            }
        }
    }
    
    setElementText(elementId, text) {
        const element = document.getElementById(elementId);
        if (element) {
            element.textContent = text;
        }
    }
    
    setElementHTML(elementId, html) {
        const element = document.getElementById(elementId);
        if (element) {
            element.innerHTML = html;
        }
    }
    
    // Navigation helpers
    redirectToLogin() {
        window.location.href = '/login';
    }
    
    redirectToDashboard() {
        window.location.href = '/dashboard';
    }
}

// Initialize framework
const esp32mvc = new ESP32MVC();

// Export for global use - ensure all methods are accessible
window.ESP32MVC = {
    // Core methods
    apiCall: (endpoint, options) => esp32mvc.apiCall(endpoint, options),
    login: (email, password) => esp32mvc.login(email, password),
    logout: () => esp32mvc.logout(),
    showAlert: (message, type, duration) => esp32mvc.showAlert(message, type, duration),
    setupForm: (formId, onSubmit) => esp32mvc.setupForm(formId, onSubmit),
    startAutoUpdate: (callback, interval) => esp32mvc.startAutoUpdate(callback, interval),
    verifyToken: () => esp32mvc.verifyToken(),
    checkAuthentication: () => esp32mvc.checkAuthentication(),
    
    // Utility methods
    formatBytes: (bytes) => esp32mvc.formatBytes(bytes),
    formatUptime: (milliseconds) => esp32mvc.formatUptime(milliseconds),
    debounce: (func, wait) => esp32mvc.debounce(func, wait),
    showModal: (modalId) => esp32mvc.showModal(modalId),
    hideModal: (modalId) => esp32mvc.hideModal(modalId),
    toggleElement: (elementId, show) => esp32mvc.toggleElement(elementId, show),
    setElementText: (elementId, text) => esp32mvc.setElementText(elementId, text),
    setElementHTML: (elementId, html) => esp32mvc.setElementHTML(elementId, html),
    
    // Properties
    apiBase: esp32mvc.apiBase,
    token: esp32mvc.token
};

// Debug: Log available methods
console.log('ESP32MVC methods:', Object.keys(window.ESP32MVC));

// Helper functions
window.showAlert = (message, type, duration) => esp32mvc.showAlert(message, type, duration);
window.apiCall = (endpoint, options) => esp32mvc.apiCall(endpoint, options);
window.logout = () => esp32mvc.logout();
window.setupForm = (formId, onSubmit) => esp32mvc.setupForm(formId, onSubmit);
window.startAutoUpdate = (callback, interval) => esp32mvc.startAutoUpdate(callback, interval);
window.formatBytes = (bytes) => esp32mvc.formatBytes(bytes);
window.formatUptime = (milliseconds) => esp32mvc.formatUptime(milliseconds);
window.debounce = (func, wait) => esp32mvc.debounce(func, wait);
window.showModal = (modalId) => esp32mvc.showModal(modalId);
window.hideModal = (modalId) => esp32mvc.hideModal(modalId);
window.toggleElement = (elementId, show) => esp32mvc.toggleElement(elementId, show);
window.setElementText = (elementId, text) => esp32mvc.setElementText(elementId, text);
window.setElementHTML = (elementId, html) => esp32mvc.setElementHTML(elementId, html);

// DOM ready helper
function ready(fn) {
    if (document.readyState !== 'loading') {
        fn();
    } else {
        document.addEventListener('DOMContentLoaded', fn);
    }
}

window.ready = ready;
