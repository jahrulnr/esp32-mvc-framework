document.addEventListener('DOMContentLoaded', function() {
		// DOM Elements
		const authWrapper = document.getElementById('auth-wrapper');
		const dashboardWrapper = document.getElementById('dashboard-wrapper');
		const loginForm = document.getElementById('login-form');
		const loginLink = document.getElementById('login-link');
		const logoutBtn = document.getElementById('logout-btn');
		const errorAlert = document.getElementById('error-alert');
		const successAlert = document.getElementById('success-alert');
		const authSubtitle = document.getElementById('auth-subtitle');
		const sidebarToggle = document.getElementById('sidebar-toggle');
		const sidebar = document.getElementById('sidebar');
		const mainContent = document.getElementById('main-content');

		// Dashboard elements
		const navLinks = document.querySelectorAll('.nav-link');
		const contentArea = mainContent;

		// Application state
		const APP_STATE = {
				auth: {
						token: localStorage.getItem('auth_token'),
						user: {
								username: localStorage.getItem('user_username'),
						}
				},
				currentView: 'login',
				currentDashboardSection: 'dashboard',
				bootTime: localStorage.getItem('boot_time') || new Date().getTime()
		};
		
		// Content templates
		const CONTENT_TEMPLATES = {
				dashboard: () => {
						const template = document.getElementById('dashboard-template');
						return template.content.cloneNode(true);
				},
				camera: () => {
						const template = document.getElementById('camera-template');
						return template.content.cloneNode(true);
				},
				servo: () => {
						const template = document.getElementById('servo-template');
						return template.content.cloneNode(true);
				},
				settings: () => {
						const template = document.getElementById('settings-template');
						return template.content.cloneNode(true);
				},
				users: () => {
						const template = document.getElementById('users-template');
						return template.content.cloneNode(true);
				},
				system: () => {
						const template = document.getElementById('system-template');
						return template.content.cloneNode(true);
				}
		};
		
		// Mobile sidebar toggle
		if (sidebarToggle) {
				sidebarToggle.addEventListener('click', function() {
						sidebar.classList.toggle('show');
						mainContent.classList.toggle('expanded');
				});
		}
		
		// Close sidebar when clicking outside on mobile
		document.addEventListener('click', function(e) {
				if (window.innerWidth <= 767.98) {
						if (!sidebar.contains(e.target) && !sidebarToggle.contains(e.target)) {
								sidebar.classList.remove('show');
								mainContent.classList.remove('expanded');
						}
				}
		});
		
		// Initialize app based on URL hash
		function initApp() {
				const hash = window.location.hash || '#login';
				
				// If authenticated, check if trying to access auth pages
				if (APP_STATE.auth.token) {
						if (hash === '#login') {
								navigateTo('#dashboard');
								return;
						}
						
						showDashboard();
						handleDashboardNavigation(hash);
				} else {
						// Not authenticated, ensure we're on auth pages
						if (hash !== '#login') {
								navigateTo('#login');
								return;
						}
						
						showAuth();
						showLoginForm();
				}
		}

		// Navigation function
		function navigateTo(hash) {
				window.location.hash = hash;
				initApp();
		}
		
		// Load content via AJAX or template
		function loadContent(sectionName) {
				// Show loading spinner
				contentArea.innerHTML = `
						<div id="content-loader" class="text-center py-5">
								<div class="spinner-border text-primary" role="status">
										<span class="visually-hidden">Loading...</span>
								</div>
						</div>
				`;
				
				// Simulate loading delay for UX
				setTimeout(() => {
						try {
								// Use template-based content loading for better performance on ESP32
								if (CONTENT_TEMPLATES[sectionName]) {
										const content = CONTENT_TEMPLATES[sectionName]();
										contentArea.innerHTML = '';
										contentArea.appendChild(content);
										
										// Initialize section-specific functionality
										initializeSectionFeatures(sectionName);
								} else {
										contentArea.innerHTML = `
												<div class="alert alert-warning">
														<h4>Section not found</h4>
														<p>The requested section "${sectionName}" could not be loaded.</p>
												</div>
										`;
								}
						} catch (error) {
								console.error('Error loading content:', error);
								contentArea.innerHTML = `
										<div class="alert alert-danger">
												<h4>Error loading content</h4>
												<p>There was an error loading the requested section. Please try again.</p>
										</div>
								`;
						}
				}, 300);
		}
		
		// Initialize section-specific features after content load
		function initializeSectionFeatures(sectionName) {
				switch (sectionName) {
						case 'dashboard':
								// Initialize dashboard features
								fetchSystemStats();
								const refreshBtn = document.getElementById('refresh-btn');
								if (refreshBtn) {
										refreshBtn.addEventListener('click', function() {
												fetchUserData();
												fetchSystemStats();
										});
								}
								break;
								
						case 'camera':
								// Initialize camera features
								initializeCameraSection();
								break;
								
						case 'servo':
								// Initialize servo features
								initializeServoSection();
								break;
								
						case 'settings':
								// Initialize settings form
								updateSettingsForm();
								const settingsForm = document.getElementById('settings-form');
								if (settingsForm) {
										settingsForm.addEventListener('submit', handlePasswordUpdate);
								}
								break;
								
						case 'users':
								// Load users data
								loadUsersTable();
								break;
								
						case 'system':
								// Initialize system controls
								fetchDetailedSystemInfo();
								const restartBtn = document.getElementById('restart-btn');
								if (restartBtn) {
										restartBtn.addEventListener('click', handleSystemRestart);
								}
								break;
				}
		}
		
		// Update settings form with user data
		function updateSettingsForm() {
				const usernameDisplay = document.getElementById('username-display');
				const roleDisplay = document.getElementById('role-display');
				
				if (usernameDisplay) {
						usernameDisplay.value = APP_STATE.auth.user.username || '';
				}
				if (roleDisplay) {
						roleDisplay.value = 'User'; // Default role
				}
		}
		
		// Load users table
		function loadUsersTable() {
				if (!APP_STATE.auth.token) return;
				
				fetch('/api/v1/admin/users', {
						headers: {
								'Authorization': 'Bearer ' + APP_STATE.auth.token
						}
				})
				.then(response => response.json())
				.then(data => {
						if (data.success) {
								const tableBody = document.getElementById('users-table-body');
								if (tableBody) {
										tableBody.innerHTML = '';
										data.users.forEach(user => {
												const row = document.createElement('tr');
												row.innerHTML = `
														<td>${user.id}</td>
														<td>${user.name || user.username}</td>
														<td>${user.username}</td>
														<td><span class="badge ${user.role === 'admin' ? 'bg-primary' : 'bg-secondary'}">${user.role}</span></td>
														<td><span class="badge ${user.active ? 'bg-success' : 'bg-danger'}">${user.active ? 'Active' : 'Inactive'}</span></td>
														<td>
																<button class="btn btn-sm btn-outline-primary">Edit</button>
																<button class="btn btn-sm btn-outline-danger">Delete</button>
														</td>
												`;
												tableBody.appendChild(row);
										});
								}
						}
				})
				.catch(error => {
						console.error('Error loading users:', error);
				});
		}
		
		// Handle password update
		function handlePasswordUpdate(e) {
				e.preventDefault();
				
				const currentPassword = document.getElementById('current-password').value;
				const newPassword = document.getElementById('new-password').value;
				const confirmPassword = document.getElementById('confirm-new-password').value;
				
				if (newPassword !== confirmPassword) {
						alert('New passwords do not match');
						return;
				}
				
				if (newPassword.length < 6) {
						alert('Password must be at least 6 characters');
						return;
				}

				const params = new URLSearchParams();
				params.current_password = currentPassword;
				params.new_password = newPassword;
				params.confirm_password = confirmPassword;
				
				// Send password update request
				fetch('/api/v1/auth/password', {
						method: 'POST',
						headers: {
								'Authorization': 'Bearer ' + APP_STATE.auth.token,
								'Content-Type': 'application/x-www-form-urlencoded'
						},
						body: params.toString()
				})
				.then(response => response.json())
				.then(data => {
						if (data.success) {
								alert('Password updated successfully');
								document.getElementById('settings-form').reset();
						} else {
								alert(data.message || 'Error updating password');
						}
				})
				.catch(error => {
						console.error('Error updating password:', error);
						alert('Error updating password');
				});
		}
		
		// Handle system restart
		function handleSystemRestart() {
				if (confirm('Are you sure you want to restart the device?')) {
						fetch('/api/v1/system/restart', {
								method: 'POST',
								headers: {
										'Authorization': 'Bearer ' + APP_STATE.auth.token,
										'Content-Type': 'application/x-www-form-urlencoded'
								}
						})
						.then(response => response.json())
						.then(data => {
								if (data.success) {
										showAlert('success', 'Device is restarting. Please wait a moment before reconnecting.');
										// Optionally redirect to login after delay
										setTimeout(() => {
												logout();
										}, 3000);
								} else {
										showAlert('error', data.message || 'Failed to restart device');
								}
						})
						.catch(error => {
								console.error('Error restarting device:', error);
								showAlert('error', 'Error restarting device');
						});
				}
		}
		
		// Handle dashboard navigation
		function handleDashboardNavigation(hash) {
				let sectionName;
				
				switch (hash) {
						case '#camera':
								sectionName = 'camera';
								break;
						case '#servo':
								sectionName = 'servo';
								break;
						case '#settings':
								sectionName = 'settings';
								break;
						case '#users':
								sectionName = 'users';
								break;
						case '#system':
								sectionName = 'system';
								break;
						default:
								sectionName = 'dashboard';
								hash = '#dashboard';
				}
				
				// Set active nav link
				navLinks.forEach(link => {
						if (link.getAttribute('href') === hash) {
								link.classList.add('active');
						} else {
								link.classList.remove('active');
						}
				});
				
				// Load content for the section
				loadContent(sectionName);
				
				APP_STATE.currentDashboardSection = sectionName;
				
				// Close mobile sidebar after navigation
				if (window.innerWidth <= 767.98) {
						sidebar.classList.remove('show');
						mainContent.classList.remove('expanded');
				}
		}
		
		// Update user info in dashboard
		function updateUserInfo() {
				document.getElementById('user-username').textContent = APP_STATE.auth.user.username || '';
		}
		
		// Fetch user data from API
		function fetchUserData() {
				if (!APP_STATE.auth.token) return;
				
				fetch('/api/v1/auth/user', {
						headers: {
								'Authorization': 'Bearer ' + APP_STATE.auth.token
						}
				})
				.then(response => response.json())
				.then(data => {
						if (data.success) {
								// Update app state
								APP_STATE.auth.user.username = data.user.username;
								
								// Save to localStorage
								localStorage.setItem('user_username', data.user.username);
								
								// Update UI
								updateUserInfo();
								
								// Show admin sections if user has permissions
								if (data.user.permissions && data.user.permissions.canManageUsers) {
										document.querySelectorAll('.admin-only').forEach(el => {
												el.style.display = 'block';
										});
								}
						} else {
								// Auth token invalid - logout
								logout();
						}
				})
				.catch(error => {
						console.error('Error fetching user data:', error);
				});
		}
		
		// Fetch system stats
		function fetchSystemStats() {
				// If not authenticated, skip API call and show mock data
				if (!APP_STATE.auth.token) {
						showMockSystemStats();
						return;
				}
				
				// Fetch real stats from API
				fetch('/api/v1/system/stats', {
						headers: {
								'Authorization': 'Bearer ' + APP_STATE.auth.token
						}
				})
				.then(response => response.json())
				.then(data => {
						if (data.success && data.data) {
								updateSystemStatsDisplay(data.data);
						} else {
								console.warn('Failed to fetch system stats, using mock data');
								showMockSystemStats();
						}
				})
				.catch(error => {
						console.error('Error fetching system stats:', error);
						showMockSystemStats();
				});
		}
		
		// Display mock system stats (fallback)
		function showMockSystemStats() {
				// Calculate uptime from boot time
				const now = new Date().getTime();
				const uptime = Math.floor((now - APP_STATE.bootTime) / 1000);
				const hours = Math.floor(uptime / 3600);
				const minutes = Math.floor((uptime % 3600) / 60);
				const seconds = uptime % 60;
				
				const uptimeElement = document.getElementById('uptime-value');
				if (uptimeElement) {
						uptimeElement.textContent = 
								`${hours.toString().padStart(2, '0')}:${minutes.toString().padStart(2, '0')}:${seconds.toString().padStart(2, '0')}`;
				}
				
				// Mock memory data
				const memory = Math.floor(Math.random() * 200) + 100;
				const memoryElement = document.getElementById('memory-value');
				const ipElement = document.getElementById('ip-value');
				
				if (memoryElement) {
						memoryElement.textContent = `${memory} KB`;
				}
				if (ipElement) {
						ipElement.textContent = '192.168.1.100';
				}
		}
		
		// Update system stats display with real data
		function updateSystemStatsDisplay(systemData) {
				// Update uptime
				const uptimeElement = document.getElementById('uptime-value');
				if (uptimeElement && systemData.uptime_formatted) {
						uptimeElement.textContent = systemData.uptime_formatted;
				}
				
				// Update memory
				const memoryElement = document.getElementById('memory-value');
				if (memoryElement && systemData.memory && systemData.memory.free_heap_formatted) {
						memoryElement.textContent = systemData.memory.free_heap_formatted;
				}
				
				// Update IP address
				const ipElement = document.getElementById('ip-value');
				if (ipElement && systemData.network && systemData.network.ip) {
						ipElement.textContent = systemData.network.ip;
				}
				
				// Update additional stats if elements exist
				const cpuElement = document.getElementById('cpu-value');
				if (cpuElement && systemData.hardware && systemData.hardware.cpu_freq) {
						cpuElement.textContent = systemData.hardware.cpu_freq + ' MHz';
				}
				
				const flashElement = document.getElementById('flash-value');
				if (flashElement && systemData.hardware && systemData.hardware.flash_size) {
						const flashMB = Math.round(systemData.hardware.flash_size / (1024 * 1024));
						flashElement.textContent = flashMB + ' MB';
				}
				
				const wifiElement = document.getElementById('wifi-value');
				if (wifiElement && systemData.network) {
						if (systemData.network.connected) {
								wifiElement.textContent = systemData.network.ssid + ' (' + systemData.network.rssi + ' dBm)';
						} else {
								wifiElement.textContent = 'Disconnected';
						}
				}
		}
		
		// Fetch detailed system information
		function fetchDetailedSystemInfo() {
				if (!APP_STATE.auth.token) return;
				
				Promise.all([
						fetch('/api/v1/system/stats', {
								headers: { 'Authorization': 'Bearer ' + APP_STATE.auth.token }
						}).then(r => r.json()),
						fetch('/api/v1/system/memory', {
								headers: { 'Authorization': 'Bearer ' + APP_STATE.auth.token }
						}).then(r => r.json()),
						fetch('/api/v1/system/network', {
								headers: { 'Authorization': 'Bearer ' + APP_STATE.auth.token }
						}).then(r => r.json())
				])
				.then(([statsData, memoryData, networkData]) => {
						updateDetailedSystemInfo(statsData.data, memoryData.memory, networkData.network);
				})
				.catch(error => {
						console.error('Error fetching detailed system info:', error);
				});
		}
		
		// Update detailed system information display
		function updateDetailedSystemInfo(stats, memory, network) {
				// Update system information panel if it exists
				const systemInfoPanel = document.getElementById('system-info-panel');
				if (!systemInfoPanel) return;
				
				let html = '<div class="row">';
				
				// Hardware Information
				if (stats && stats.hardware) {
						html += `
								<div class="col-md-6 mb-3">
										<div class="card">
												<div class="card-header"><h6 class="mb-0">Hardware Information</h6></div>
												<div class="card-body">
														<small class="text-muted">Chip:</small> ${stats.hardware.chip_model} Rev ${stats.hardware.chip_revision}<br>
														<small class="text-muted">Cores:</small> ${stats.hardware.chip_cores}<br>
														<small class="text-muted">CPU Frequency:</small> ${stats.hardware.cpu_freq} MHz<br>
														<small class="text-muted">Flash Size:</small> ${Math.round(stats.hardware.flash_size / (1024*1024))} MB<br>
														<small class="text-muted">Flash Speed:</small> ${Math.round(stats.hardware.flash_speed / 1000000)} MHz
												</div>
										</div>
								</div>
						`;
				}
				
				// Memory Information
				if (memory) {
						const heapUsagePercent = ((memory.used_heap / memory.total_heap) * 100).toFixed(1);
						html += `
								<div class="col-md-6 mb-3">
										<div class="card">
												<div class="card-header"><h6 class="mb-0">Memory Information</h6></div>
												<div class="card-body">
														<small class="text-muted">Free Heap:</small> ${formatBytes(memory.free_heap)}<br>
														<small class="text-muted">Used Heap:</small> ${formatBytes(memory.used_heap)} (${heapUsagePercent}%)<br>
														<small class="text-muted">Largest Block:</small> ${formatBytes(memory.largest_free_block)}<br>
														${memory.psram_found ? `<small class="text-muted">PSRAM:</small> ${formatBytes(memory.free_psram)} free<br>` : ''}
														<small class="text-muted">Fragmentation:</small> ${memory.heap_fragmentation ? memory.heap_fragmentation.toFixed(1) + '%' : 'N/A'}
												</div>
										</div>
								</div>
						`;
				}
				
				// Network Information
				if (network) {
						html += `
								<div class="col-md-6 mb-3">
										<div class="card">
												<div class="card-header"><h6 class="mb-0">Network Information</h6></div>
												<div class="card-body">
														${network.connected ? `
																<small class="text-muted">SSID:</small> ${network.ssid}<br>
																<small class="text-muted">IP Address:</small> ${network.ip}<br>
																<small class="text-muted">Gateway:</small> ${network.gateway}<br>
																<small class="text-muted">Signal:</small> ${network.rssi} dBm<br>
																<small class="text-muted">MAC:</small> ${network.mac}
														` : '<span class="text-danger">Not Connected</span>'}
												</div>
										</div>
								</div>
						`;
				}
				
				// Software Information
				if (stats && stats.software) {
						html += `
								<div class="col-md-6 mb-3">
										<div class="card">
												<div class="card-header"><h6 class="mb-0">Software Information</h6></div>
												<div class="card-body">
														<small class="text-muted">SDK Version:</small> ${stats.software.sdk_version}<br>
														<small class="text-muted">Arduino:</small> ${stats.software.arduino_version}<br>
														<small class="text-muted">Compiled:</small> ${stats.software.compile_date} ${stats.software.compile_time}<br>
														<small class="text-muted">Uptime:</small> ${stats.uptime_formatted}
												</div>
										</div>
								</div>
						`;
				}
				
				html += '</div>';
				systemInfoPanel.innerHTML = html;
		}
		
		// Show login form
		function showLoginForm() {
				loginForm.style.display = 'block';
				loginLink.style.display = 'none';
				authSubtitle.textContent = 'Please sign in to continue';
				APP_STATE.currentView = 'login';
		}
		
		// Show auth view (login)
		function showAuth() {
				authWrapper.style.display = 'flex';
				dashboardWrapper.style.display = 'none';
				document.body.classList.add('auth-body');
				document.body.classList.remove('dashboard-body');
		}
		
		// Show dashboard view
		function showDashboard() {
				authWrapper.style.display = 'none';
				dashboardWrapper.style.display = 'block';
				document.body.classList.remove('auth-body');
				document.body.classList.add('dashboard-body');
				
				// Update user info in dashboard
				updateUserInfo();
				
				// Fetch initial stats and user data
				fetchUserData();
		}
		
		// Logout function
		function logout() {
				fetch('/logout', {
						method: 'POST',
						headers: {
								'Authorization': 'Bearer ' + APP_STATE.auth.token,
								'Content-Type': 'application/x-www-form-urlencoded'
						}
				})
				.catch(error => console.error('Logout error:', error))
				.finally(() => {
						// Clear auth state regardless of server response
						localStorage.removeItem('auth_token');
						localStorage.removeItem('user_username');
						
						APP_STATE.auth.token = null;
						APP_STATE.auth.user = {};
						
						// Redirect to login
						navigateTo('#login');
				});
		}

		// Event Listeners
		
		// Hash change listener
		window.addEventListener('hashchange', initApp);
		
		// Login form submit
		loginForm.addEventListener('submit', function(e) {
				e.preventDefault();
				
				const username = document.getElementById('username').value;
				const password = document.getElementById('password').value;
				const redirect = document.getElementById('redirect').value;
				
				// Reset alerts
				errorAlert.style.display = 'none';
				successAlert.style.display = 'none';
				
				// Send login request
				fetch('/login', {
						method: 'POST',
						headers: {
								'Content-Type': 'application/x-www-form-urlencoded',
						},
						body: `username=${encodeURIComponent(username)}&password=${encodeURIComponent(password)}&redirect=${encodeURIComponent(redirect)}`
				})
				.then(response => response.json())
				.then(data => {
						if (data.success) {
								// Show success message
								successAlert.textContent = data.message;
								successAlert.style.display = 'block';
								
								// Update app state
								APP_STATE.auth.token = data.token;
								APP_STATE.auth.user.username = data.user.username;
								
								// Store auth data
								localStorage.setItem('auth_token', data.token);
								localStorage.setItem('user_username', data.user.username);
								
								// Navigate to dashboard after short delay
								setTimeout(() => {
										navigateTo('#dashboard');
								}, 1000);
						} else {
								// Show error message
								errorAlert.textContent = data.message;
								errorAlert.style.display = 'block';
						}
				})
				.catch(error => {
						console.error('Error:', error);
						errorAlert.textContent = 'An error occurred. Please try again.';
						errorAlert.style.display = 'block';
				});
		});
		
		loginLink.querySelector('a').addEventListener('click', function(e) {
				e.preventDefault();
				navigateTo('#login');
		});
		
		// Camera WebSocket and functionality
		let cameraWs = null;
		let cameraStats = {
				framesReceived: 0,
				dataReceived: 0,
				lastFrameSize: 0,
				fpsCounter: 0,
				lastFpsUpdate: Date.now()
		};
		
		function initializeCameraSection() {
				// Get camera elements
				const startBtn = document.getElementById('camera-start-btn');
				const stopBtn = document.getElementById('camera-stop-btn');
				const captureBtn = document.getElementById('camera-capture-btn');
				const settingsForm = document.getElementById('camera-settings-form');
				const applySettingsBtn = document.getElementById('apply-settings-btn');
				
				// Range inputs for real-time updates
				const qualitySlider = document.getElementById('camera-quality');
				const brightnessSlider = document.getElementById('camera-brightness');
				const contrastSlider = document.getElementById('camera-contrast');
				
				// Add event listeners
				if (startBtn) startBtn.addEventListener('click', startCameraStream);
				if (stopBtn) stopBtn.addEventListener('click', stopCameraStream);
				if (captureBtn) captureBtn.addEventListener('click', capturePhoto);
				if (settingsForm) settingsForm.addEventListener('submit', applyCameraSettings);
				
				// Real-time slider updates
				if (qualitySlider) {
						qualitySlider.addEventListener('input', (e) => {
								document.getElementById('quality-value').textContent = e.target.value;
						});
				}
				if (brightnessSlider) {
						brightnessSlider.addEventListener('input', (e) => {
								document.getElementById('brightness-value').textContent = e.target.value;
						});
				}
				if (contrastSlider) {
						contrastSlider.addEventListener('input', (e) => {
								document.getElementById('contrast-value').textContent = e.target.value;
						});
				}
				
				// Load current camera settings
				loadCameraSettings();
		}
		
		function startCameraStream() {
				const startBtn = document.getElementById('camera-start-btn');
				const stopBtn = document.getElementById('camera-stop-btn');
				const captureBtn = document.getElementById('camera-capture-btn');
				const applySettingsBtn = document.getElementById('apply-settings-btn');
				const statusBadge = document.getElementById('camera-status');
				const connectionStatus = document.getElementById('connection-status');
				const cameraMessage = document.getElementById('camera-message');
				const cameraLoading = document.getElementById('camera-loading');
				
				if (cameraWs && cameraWs.readyState === WebSocket.OPEN) {
						return;
				}
				
				// Update UI
				startBtn.disabled = true;
				cameraLoading.style.display = 'block';
				cameraMessage.innerHTML = '<h4>Connecting...</h4><p>Establishing camera connection</p>';
				
				// Connect to camera WebSocket
				const wsUrl = `ws://${window.location.host}/ws/camera`;
				cameraWs = new WebSocket(wsUrl);
				
				cameraWs.onopen = function() {
						console.log('Camera WebSocket connected');
						
						// Update UI
						statusBadge.textContent = 'Connected';
						statusBadge.className = 'badge bg-success';
						connectionStatus.textContent = 'Connected';
						connectionStatus.className = 'badge bg-success';
						
						startBtn.disabled = true;
						stopBtn.disabled = false;
						captureBtn.disabled = false;
						
						cameraLoading.style.display = 'none';
						cameraMessage.innerHTML = '<h4>Camera Ready</h4><p>Stream will appear here</p>';
						
						// Start requesting frames
						requestCameraFrame();
				};
				
				cameraWs.onmessage = function(event) {
						if (event.data instanceof Blob) {
								// Binary frame data
								handleCameraFrame(event.data);
						} else {
								// Text message (JSON)
								try {
										const data = JSON.parse(event.data);
										console.log('Camera message:', data);
								} catch (e) {
										console.log('Camera text:', event.data);
								}
						}
				};
				
				cameraWs.onclose = function() {
						console.log('Camera WebSocket disconnected');
						updateCameraDisconnectedState();
				};
				
				cameraWs.onerror = function(error) {
						console.error('Camera WebSocket error:', error);
						updateCameraDisconnectedState();
				};
		}
		
		function stopCameraStream() {
				if (cameraWs) {
						cameraWs.close();
				}
				updateCameraDisconnectedState();
		}
		
		function updateCameraDisconnectedState() {
				const startBtn = document.getElementById('camera-start-btn');
				const stopBtn = document.getElementById('camera-stop-btn');
				const captureBtn = document.getElementById('camera-capture-btn');
				const applySettingsBtn = document.getElementById('apply-settings-btn');
				const statusBadge = document.getElementById('camera-status');
				const connectionStatus = document.getElementById('connection-status');
				const cameraMessage = document.getElementById('camera-message');
				const cameraLoading = document.getElementById('camera-loading');
				const fpsCounter = document.getElementById('fps-counter');
				
				// Update UI
				statusBadge.textContent = 'Disconnected';
				statusBadge.className = 'badge bg-secondary';
				connectionStatus.textContent = 'Disconnected';
				connectionStatus.className = 'badge bg-secondary';
				fpsCounter.textContent = '0 FPS';
				
				startBtn.disabled = false;
				stopBtn.disabled = true;
				captureBtn.disabled = true;
				
				cameraLoading.style.display = 'none';
				cameraMessage.innerHTML = '<h4>Camera Offline</h4><p>Click "Start Camera" to begin streaming</p>';
				
				// Reset stats
				cameraStats.framesReceived = 0;
				cameraStats.dataReceived = 0;
				cameraStats.fpsCounter = 0;
				updateCameraStats();
		}
		
		function requestCameraFrame() {
				if (cameraWs && cameraWs.readyState === WebSocket.OPEN) {
						const command = {
								command: 'capture'
						};
						cameraWs.send(JSON.stringify(command));
						
						// Request next frame after a delay (streaming)
						setTimeout(requestCameraFrame, 100); // ~10 FPS
				}
		}
		
		function handleCameraFrame(blob) {
				const cameraImg = document.getElementById('camera-stream');
				const cameraMessage = document.getElementById('camera-message');
				
				if (cameraImg) {
						// Create object URL for the blob
						const url = URL.createObjectURL(blob);
						
						// Update image
						const oldUrl = cameraImg.src;
						cameraImg.src = url;
						cameraImg.style.display = 'block';
						
						// Hide message overlay
						cameraMessage.style.display = 'none';
						
						// Clean up previous URL
						if (oldUrl && oldUrl.startsWith('blob:')) {
								URL.revokeObjectURL(oldUrl);
						}
						
						// Update statistics
						cameraStats.framesReceived++;
						cameraStats.lastFrameSize = blob.size;
						cameraStats.dataReceived += blob.size;
						
						// Calculate FPS
						const now = Date.now();
						if (now - cameraStats.lastFpsUpdate >= 1000) {
								cameraStats.fpsCounter = Math.round(cameraStats.framesReceived / ((now - cameraStats.lastFpsUpdate) / 1000));
								cameraStats.lastFpsUpdate = now;
								cameraStats.framesReceived = 0;
						}
						
						updateCameraStats();
				}
		}
		
		function updateCameraStats() {
				const framesEl = document.getElementById('frames-received');
				const dataEl = document.getElementById('data-received');
				const frameSizeEl = document.getElementById('last-frame-size');
				const fpsEl = document.getElementById('fps-counter');
				
				if (framesEl) framesEl.textContent = cameraStats.framesReceived;
				if (dataEl) dataEl.textContent = formatBytes(cameraStats.dataReceived);
				if (frameSizeEl) frameSizeEl.textContent = formatBytes(cameraStats.lastFrameSize);
				if (fpsEl) fpsEl.textContent = `${cameraStats.fpsCounter} FPS`;
		}
		
		function capturePhoto() {
				// Single capture via HTTP API
				fetch('/api/v1/camera/capture', {
						method: 'POST',
						headers: {
								'Authorization': `Bearer ${APP_STATE.auth.token}`,
								'Content-Type': 'application/x-www-form-urlencoded'
						}
				})
				.then(response => {
						if (response.ok) {
								return response.blob();
						}
						throw new Error('Capture failed');
				})
				.then(blob => {
						// Download the captured image
						const url = URL.createObjectURL(blob);
						const a = document.createElement('a');
						a.href = url;
						a.download = `capture_${new Date().getTime()}.jpg`;
						a.click();
						URL.revokeObjectURL(url);
				})
				.catch(error => {
						console.error('Capture error:', error);
						showAlert('error', 'Failed to capture photo');
				});
		}
		
		function applyCameraSettings(event) {
				event.preventDefault();
				
				const quality = document.getElementById('camera-quality').value;
				const brightness = document.getElementById('camera-brightness').value;
				const contrast = document.getElementById('camera-contrast').value;
				const framesize = document.getElementById('camera-framesize').value;
				
				const params = new URLSearchParams();
				params.append('quality', quality);
				params.append('brightness', brightness);
				params.append('contrast', contrast);
				params.append('framesize', framesize);

				fetch('/api/v1/camera/settings', {
						method: 'POST',
						headers: {
								'Authorization': `Bearer ${APP_STATE.auth.token}`,
								'Content-Type': 'application/x-www-form-urlencoded',
						},
						body: params.toString()
				})
				.then(response => response.json())
				.then(data => {
						if (data.success) {
								showAlert('success', 'Camera settings updated successfully');
						} else {
								showAlert('error', 'Failed to update camera settings');
						}
				})
				.catch(error => {
						console.error('Settings error:', error);
						showAlert('error', 'Failed to update camera settings');
				});
		}
		
		function loadCameraSettings() {
				fetch('/api/v1/camera/settings', {
						method: 'GET',
						headers: {
								'Authorization': `Bearer ${APP_STATE.auth.token}`
						}
				})
				.then(response => response.json())
				.then(data => {
						if (data.success && data.settings) {
								const settings = data.settings;
								
								// Update form values
								if (settings.quality !== undefined) {
										document.getElementById('camera-quality').value = settings.quality;
										document.getElementById('quality-value').textContent = settings.quality;
								}
								if (settings.brightness !== undefined) {
										document.getElementById('camera-brightness').value = settings.brightness;
										document.getElementById('brightness-value').textContent = settings.brightness;
								}
								if (settings.contrast !== undefined) {
										document.getElementById('camera-contrast').value = settings.contrast;
										document.getElementById('contrast-value').textContent = settings.contrast;
								}
								if (settings.framesize !== undefined) {
										document.getElementById('camera-framesize').value = settings.framesize;
								}
						}
				})
				.catch(error => {
						console.error('Load settings error:', error);
				});
		}
		
		// ============ SERVO CONTROL FUNCTIONS ============
		
		// Servo state management
		const SERVO_STATE = {
				servos: {},
				presets: {},
				currentStats: {
						total: 0,
						active: 0,
						savedPresets: 0
				}
		};
		
		// Helper function to get the preferred identifier for a servo
		// Returns name if available and not empty, otherwise returns pin number as string
		function getServoIdentifier(servo) {
				return (servo.name && servo.name.trim()) ? servo.name : servo.pin.toString();
		}
		
		function initializeServoSection() {
				// Initialize servo controls
				const addServoBtn = document.getElementById('add-servo-btn');
				const setupPanTiltBtn = document.getElementById('setup-pantilt-btn');
				const savePresetBtn = document.getElementById('save-preset-btn');
				const savePresetBtnPanel = document.getElementById('save-preset-btn-panel');
				const loadPresetsBtn = document.getElementById('load-presets-btn');
				const setAllBtn = document.getElementById('set-all-btn');
				const centerAllBtn = document.getElementById('center-all-btn');
				const sweepAllBtn = document.getElementById('sweep-all-btn');
				const disableAllBtn = document.getElementById('disable-all-btn');
				
				// Event listeners
				if (addServoBtn) addServoBtn.addEventListener('click', showAddServoModal);
				if (setupPanTiltBtn) setupPanTiltBtn.addEventListener('click', showPanTiltSetupModal);
				if (savePresetBtn) savePresetBtn.addEventListener('click', showSavePresetModal);
				if (savePresetBtnPanel) savePresetBtnPanel.addEventListener('click', showSavePresetModal);
				if (loadPresetsBtn) loadPresetsBtn.addEventListener('click', loadPresets);
				if (setAllBtn) setAllBtn.addEventListener('click', setAllServos);
				if (centerAllBtn) centerAllBtn.addEventListener('click', () => setAllServosAngle(90));
				if (sweepAllBtn) sweepAllBtn.addEventListener('click', sweepAllServos);
				if (disableAllBtn) disableAllBtn.addEventListener('click', disableAllServos);
				
				// Load initial servo data
				loadServoList();
				loadPresets();
		}
		
		function loadServoList() {
				fetch('/api/v1/servo/list')
						.then(response => response.json())
						.then(data => {
								if (data.success) {
										// Convert array to object with pin as key for easier lookup
										SERVO_STATE.servos = {};
										if (data.servos && Array.isArray(data.servos)) {
												data.servos.forEach(servo => {
														SERVO_STATE.servos[servo.pin] = servo;
												});
										}
										updateServoDisplay();
										updateServoStats();
								}
						})
						.catch(error => {
								console.error('Error loading servos:', error);
								showAlert('danger', 'Error loading servo list');
						});
		}
		
		function updateServoDisplay() {
				const servoList = document.getElementById('servo-list');
				const noServosMessage = document.getElementById('no-servos-message');
				const bulkControlsCard = document.getElementById('bulk-controls-card');
				
				if (!servoList) return;
				
				const servoCount = Object.keys(SERVO_STATE.servos).length;
				
				if (servoCount === 0) {
						servoList.innerHTML = '<div class="col-12 text-center text-muted" id="no-servos-message"><div class="py-5"><svg width="64" height="64" viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg" style="opacity: 0.3;" class="mb-3"><path d="M12 2L13.09 8.26L22 9L13.09 9.74L12 16L10.91 9.74L2 9L10.91 8.26L12 2Z" fill="currentColor"/><circle cx="12" cy="12" r="3" fill="currentColor"/></svg><h5>No Servos Configured</h5><p class="text-muted">Add a servo to get started with motor control</p><button type="button" class="btn btn-primary btn-sm" onclick="document.getElementById(\'add-servo-btn\').click()">Add Your First Servo</button></div></div>';
						if (bulkControlsCard) bulkControlsCard.style.display = 'none';
						return;
				}
				
				// Show bulk controls
				if (bulkControlsCard) bulkControlsCard.style.display = 'block';
				
				// Clear and populate servo list
				servoList.innerHTML = '';
				
				Object.entries(SERVO_STATE.servos).forEach(([pin, servo]) => {
						const servoCard = createServoCard(pin, servo);
						servoList.appendChild(servoCard);
				});
		}
		
		function createServoCard(pin, servo) {
				const col = document.createElement('div');
				col.className = 'col-md-6 col-xl-4';
				
				// Use pin as primary identifier, but display name if available
				const displayName = servo.name || `Servo Pin ${pin}`;
				const identifier = getServoIdentifier(servo); // Use helper function
				
				col.innerHTML = `
						<div class="card servo-card h-100" data-servo="${pin}">
								<div class="card-header d-flex justify-content-between align-items-center">
										<h6 class="mb-0">${displayName}</h6>
										<span class="badge ${servo.is_enabled ? 'bg-success' : 'bg-secondary'}">${servo.is_enabled ? 'Active' : 'Disabled'}</span>
								</div>
								<div class="card-body">
										<div class="mb-3">
												<small class="text-muted">Pin: ${servo.pin} | Range: ${servo.min_angle || 0}°-${servo.max_angle || 180}°</small>
										</div>
										<div class="mb-3">
												<label class="form-label d-flex justify-content-between">
														<span>Angle</span>
														<span class="badge bg-primary">${servo.current_angle || 90}°</span>
												</label>
												<input type="range" class="form-range servo-angle-slider" 
													   min="${servo.min_angle || 0}" max="${servo.max_angle || 180}" 
													   value="${servo.current_angle || 90}" 
													   data-servo="${pin}">
										</div>
										<div class="d-flex gap-2 flex-wrap">
												<button class="btn btn-sm btn-outline-primary servo-set-btn" data-servo="${pin}">
														Set
												</button>
												<button class="btn btn-sm btn-outline-secondary servo-center-btn" data-servo="${pin}">
														Center
												</button>
												<button class="btn btn-sm btn-outline-warning servo-sweep-btn" data-servo="${pin}">
														Sweep
												</button>
												<button class="btn btn-sm btn-outline-danger servo-remove-btn" data-servo="${pin}">
														Remove
												</button>
										</div>
								</div>
						</div>
				`;
				
				// Add event listeners
				const angleSlider = col.querySelector('.servo-angle-slider');
				const setBtn = col.querySelector('.servo-set-btn');
				const centerBtn = col.querySelector('.servo-center-btn');
				const sweepBtn = col.querySelector('.servo-sweep-btn');
				const removeBtn = col.querySelector('.servo-remove-btn');
				
				angleSlider.addEventListener('input', (e) => {
						const angle = e.target.value;
						const badge = col.querySelector('.badge.bg-primary');
						badge.textContent = `${angle}°`;
				});
				
				setBtn.addEventListener('click', () => setServoAngle(identifier, angleSlider.value));
				centerBtn.addEventListener('click', () => setServoAngle(identifier, 90));
				sweepBtn.addEventListener('click', () => sweepServo(identifier));
				removeBtn.addEventListener('click', () => removeServo(identifier));
				
				return col;
		}
		
		function setServoAngle(identifier, angle, smooth = false) {
				const endpoint = smooth ? '/api/v1/servo/angle/smooth' : '/api/v1/servo/angle';
				const params = new URLSearchParams();
				params.append('identifier', identifier);
				params.append('angle', angle);
				if (smooth) params.append('delay', '15');
				
				fetch(endpoint, {
						method: 'POST',
						headers: {
								'Content-Type': 'application/x-www-form-urlencoded'
						},
						body: params
				})
				.then(response => response.json())
				.then(data => {
						if (data.success) {
								// Update local state - find servo by pin or name
								Object.keys(SERVO_STATE.servos).forEach(pin => {
										const servo = SERVO_STATE.servos[pin];
										if (pin == identifier || servo.name === identifier) {
												servo.current_angle = parseInt(angle);
										}
								});
								showAlert('success', `Servo ${identifier} moved to ${angle}°`);
						} else {
								showAlert('danger', `Error: ${data.message}`);
						}
				})
				.catch(error => {
						console.error('Error setting servo angle:', error);
						showAlert('danger', 'Error setting servo angle');
				});
		}
		
		function sweepServo(identifier) {
				const params = new URLSearchParams();
				params.append('identifier', identifier);
				
				fetch('/api/v1/servo/sweep', {
						method: 'POST',
						headers: {
								'Content-Type': 'application/x-www-form-urlencoded'
						},
						body: params
				})
				.then(response => response.json())
				.then(data => {
						if (data.success) {
								showAlert('info', `Servo ${identifier} sweep started`);
						} else {
								showAlert('danger', `Error: ${data.message}`);
						}
				})
				.catch(error => {
						console.error('Error sweeping servo:', error);
						showAlert('danger', 'Error sweeping servo');
				});
		}
		
		function removeServo(identifier) {
				if (!confirm(`Are you sure you want to remove servo ${identifier}?`)) return;
				
				const params = new URLSearchParams();
				params.append('identifier', identifier);
				
				fetch('/api/v1/servo/remove', {
						method: 'POST',
						headers: {
								'Content-Type': 'application/x-www-form-urlencoded'
						},
						body: params
				})
				.then(response => response.json())
				.then(data => {
						if (data.success) {
								// Remove from local state - find servo by pin or name
								Object.keys(SERVO_STATE.servos).forEach(pin => {
										const servo = SERVO_STATE.servos[pin];
										if (pin == identifier || servo.name === identifier) {
												delete SERVO_STATE.servos[pin];
										}
								});
								updateServoDisplay();
								updateServoStats();
								showAlert('success', `Servo ${identifier} removed`);
						} else {
								showAlert('danger', `Error: ${data.message}`);
						}
				})
				.catch(error => {
						console.error('Error removing servo:', error);
						showAlert('danger', 'Error removing servo');
				});
		}
		
		function setAllServos() {
				const angle = document.getElementById('bulk-angle').value;
				setAllServosAngle(angle);
		}
		
		function setAllServosAngle(angle) {
				const params = new URLSearchParams();
				params.append('angle', angle);
				
				fetch('/api/v1/servo/set-all', {
						method: 'POST',
						headers: {
								'Content-Type': 'application/x-www-form-urlencoded'
						},
						body: params
				})
				.then(response => response.json())
				.then(data => {
						if (data.success) {
								loadServoList(); // Refresh the display
								showAlert('success', `All servos set to ${angle}°`);
						} else {
								showAlert('danger', `Error: ${data.message}`);
						}
				})
				.catch(error => {
						console.error('Error setting all servos:', error);
						showAlert('danger', 'Error setting all servos');
				});
		}
		
		function sweepAllServos() {
				fetch('/api/v1/servo/sweep-all', {
						method: 'POST'
				})
				.then(response => response.json())
				.then(data => {
						if (data.success) {
								showAlert('info', 'All servos sweep started');
						} else {
								showAlert('danger', `Error: ${data.message}`);
						}
				})
				.catch(error => {
						console.error('Error sweeping all servos:', error);
						showAlert('danger', 'Error sweeping all servos');
				});
		}
		
		function disableAllServos() {
				if (!confirm('Are you sure you want to disable all servos?')) return;
				
				fetch('/api/v1/servo/disable-all', {
						method: 'POST'
				})
				.then(response => response.json())
				.then(data => {
						if (data.success) {
								loadServoList(); // Refresh the display
								showAlert('warning', 'All servos disabled');
						} else {
								showAlert('danger', `Error: ${data.message}`);
						}
				})
				.catch(error => {
						console.error('Error disabling all servos:', error);
						showAlert('danger', 'Error disabling all servos');
				});
		}
		
		function updateServoStats() {
				const servos = Object.values(SERVO_STATE.servos);
				const stats = {
						total: servos.length,
						active: servos.filter(s => s.enabled).length,
						savedPresets: Object.keys(SERVO_STATE.presets).length
				};
				
				// Update display
				const totalElement = document.getElementById('total-servos');
				const activeElement = document.getElementById('active-servos');
				const presetsElement = document.getElementById('saved-presets');
				const countElement = document.getElementById('servo-count');
				
				if (totalElement) totalElement.textContent = stats.total;
				if (activeElement) activeElement.textContent = stats.active;
				if (presetsElement) presetsElement.textContent = stats.savedPresets;
				if (countElement) countElement.textContent = `${stats.total} servo${stats.total !== 1 ? 's' : ''}`;
		}
		
		function showAddServoModal() {
				// Create modal dynamically
				const modal = document.createElement('div');
				modal.className = 'modal fade';
				modal.innerHTML = `
						<div class="modal-dialog">
								<div class="modal-content">
										<div class="modal-header">
												<h5 class="modal-title">Add New Servo</h5>
												<button type="button" class="btn-close" data-bs-dismiss="modal"></button>
										</div>
										<form id="add-servo-form">
												<div class="modal-body">
														<div class="mb-3">
																<label for="servo-pin" class="form-label">Pin Number *</label>
																<input type="number" class="form-control" id="servo-pin" min="0" max="39" required>
																<small class="form-text text-muted">ESP32 pin number (0-39)</small>
														</div>
														<div class="mb-3">
																<label for="servo-name" class="form-label">Name (Optional)</label>
																<input type="text" class="form-control" id="servo-name" placeholder="e.g., Pan Servo">
														</div>
														<div class="row">
																<div class="col-6">
																		<label for="servo-min-angle" class="form-label">Min Angle</label>
																		<input type="number" class="form-control" id="servo-min-angle" value="0" min="0" max="180">
																</div>
																<div class="col-6">
																		<label for="servo-max-angle" class="form-label">Max Angle</label>
																		<input type="number" class="form-control" id="servo-max-angle" value="180" min="0" max="180">
																</div>
														</div>
												</div>
												<div class="modal-footer">
														<button type="button" class="btn btn-secondary" data-bs-dismiss="modal">Cancel</button>
														<button type="submit" class="btn btn-primary">Add Servo</button>
												</div>
										</form>
								</div>
						</div>
				`;
				
				document.body.appendChild(modal);
				const bsModal = new bootstrap.Modal(modal);
				bsModal.show();
				
				// Handle form submission
				const form = modal.querySelector('#add-servo-form');
				form.addEventListener('submit', (e) => {
						e.preventDefault();
						const pin = document.getElementById('servo-pin').value;
						const name = document.getElementById('servo-name').value;
						const minAngle = document.getElementById('servo-min-angle').value;
						const maxAngle = document.getElementById('servo-max-angle').value;
						
						addServo(pin, name, minAngle, maxAngle);
						bsModal.hide();
				});
				
				// Clean up modal when hidden
				modal.addEventListener('hidden.bs.modal', () => {
						modal.remove();
				});
		}
		
		function addServo(pin, name, minAngle, maxAngle) {
				const params = new URLSearchParams();
				params.append('pin', pin);
				if (name) params.append('name', name);
				params.append('min_angle', minAngle);
				params.append('max_angle', maxAngle);
				
				fetch('/api/v1/servo/add', {
						method: 'POST',
						headers: {
								'Content-Type': 'application/x-www-form-urlencoded'
						},
						body: params
				})
				.then(response => response.json())
				.then(data => {
						if (data.success) {
								loadServoList(); // Refresh the display
								showAlert('success', `Servo added on pin ${pin}`);
						} else {
								showAlert('danger', `Error: ${data.message}`);
						}
				})
				.catch(error => {
						console.error('Error adding servo:', error);
						showAlert('danger', 'Error adding servo');
				});
		}
		
		function showPanTiltSetupModal() {
				// Create modal for quick pan-tilt setup
				const modal = document.createElement('div');
				modal.className = 'modal fade';
				modal.innerHTML = `
						<div class="modal-dialog">
								<div class="modal-content">
										<div class="modal-header">
												<h5 class="modal-title">Quick Pan-Tilt Setup</h5>
												<button type="button" class="btn-close" data-bs-dismiss="modal"></button>
										</div>
										<form id="pantilt-setup-form">
												<div class="modal-body">
														<div class="alert alert-info">
																<small>This will quickly set up two servos for pan-tilt camera control.</small>
														</div>
														<div class="row">
																<div class="col-6">
																		<label for="pan-pin" class="form-label">Pan Pin *</label>
																		<input type="number" class="form-control" id="pan-pin" min="0" max="39" value="12" required>
																</div>
																<div class="col-6">
																		<label for="tilt-pin" class="form-label">Tilt Pin *</label>
																		<input type="number" class="form-control" id="tilt-pin" min="0" max="39" value="13" required>
																</div>
														</div>
												</div>
												<div class="modal-footer">
														<button type="button" class="btn btn-secondary" data-bs-dismiss="modal">Cancel</button>
														<button type="submit" class="btn btn-primary">Setup Pan-Tilt</button>
												</div>
										</form>
								</div>
						</div>
				`;
				
				document.body.appendChild(modal);
				const bsModal = new bootstrap.Modal(modal);
				bsModal.show();
				
				// Handle form submission
				const form = modal.querySelector('#pantilt-setup-form');
				form.addEventListener('submit', (e) => {
						e.preventDefault();
						const panPin = document.getElementById('pan-pin').value;
						const tiltPin = document.getElementById('tilt-pin').value;
						
						setupPanTilt(panPin, tiltPin);
						bsModal.hide();
				});
				
				// Clean up modal when hidden
				modal.addEventListener('hidden.bs.modal', () => {
						modal.remove();
				});
		}
		
		function setupPanTilt(panPin, tiltPin) {
				const params = new URLSearchParams();
				params.append('pan_pin', panPin);
				params.append('tilt_pin', tiltPin);
				
				fetch('/api/v1/servo/setup/pantilt', {
						method: 'POST',
						headers: {
								'Content-Type': 'application/x-www-form-urlencoded'
						},
						body: params
				})
				.then(response => response.json())
				.then(data => {
						if (data.success) {
								loadServoList(); // Refresh the display
								showAlert('success', `Pan-tilt setup complete on pins ${panPin} and ${tiltPin}`);
						} else {
								showAlert('danger', `Error: ${data.message}`);
						}
				})
				.catch(error => {
						console.error('Error setting up pan-tilt:', error);
						showAlert('danger', 'Error setting up pan-tilt');
				});
		}
		
		function showSavePresetModal() {
				const presetName = document.getElementById('preset-name').value;
				if (!presetName.trim()) {
						showAlert('warning', 'Please enter a preset name');
						return;
				}
				
				savePreset(presetName.trim());
		}
		
		function savePreset(name) {
				const params = new URLSearchParams();
				params.append('name', name);
				
				fetch('/api/v1/servo/preset/save', {
						method: 'POST',
						headers: {
								'Content-Type': 'application/x-www-form-urlencoded'
						},
						body: params
				})
				.then(response => response.json())
				.then(data => {
						if (data.success) {
								document.getElementById('preset-name').value = '';
								loadPresets();
								showAlert('success', `Preset "${name}" saved`);
						} else {
								showAlert('danger', `Error: ${data.message}`);
						}
				})
				.catch(error => {
						console.error('Error saving preset:', error);
						showAlert('danger', 'Error saving preset');
				});
		}
		
		function loadPresets() {
				fetch('/api/v1/servo/preset/list')
						.then(response => response.json())
						.then(data => {
								if (data.success) {
										SERVO_STATE.presets = data.presets || {};
										updatePresetDisplay();
										updateServoStats();
								}
						})
						.catch(error => {
								console.error('Error loading presets:', error);
						});
		}
		
		function updatePresetDisplay() {
				const presetList = document.getElementById('preset-list');
				if (!presetList) return;
				
				const presetCount = Object.keys(SERVO_STATE.presets).length;
				
				if (presetCount === 0) {
						presetList.innerHTML = '<small class="text-muted">No presets saved yet</small>';
						return;
				}
				
				presetList.innerHTML = Object.entries(SERVO_STATE.presets).map(([name, preset]) => `
						<div class="d-flex justify-content-between align-items-center mb-2 p-2 border rounded">
								<span class="small">${name}</span>
								<div>
										<button class="btn btn-sm btn-outline-primary me-1" onclick="loadPreset('${name}')">Load</button>
										<button class="btn btn-sm btn-outline-danger" onclick="deletePreset('${name}')">×</button>
								</div>
						</div>
				`).join('');
		}
		
		function loadPreset(name) {
				const params = new URLSearchParams();
				params.append('name', name);
				
				fetch('/api/v1/servo/preset/load', {
						method: 'POST',
						headers: {
								'Content-Type': 'application/x-www-form-urlencoded'
						},
						body: params
				})
				.then(response => response.json())
				.then(data => {
						if (data.success) {
								loadServoList(); // Refresh the display
								showAlert('success', `Preset "${name}" loaded`);
						} else {
								showAlert('danger', `Error: ${data.message}`);
						}
				})
				.catch(error => {
						console.error('Error loading preset:', error);
						showAlert('danger', 'Error loading preset');
				});
		}
		
		function deletePreset(name) {
				if (!confirm(`Are you sure you want to delete preset "${name}"?`)) return;
				
				const params = new URLSearchParams();
				params.append('name', name);
				
				fetch('/api/v1/servo/preset/delete', {
						method: 'POST',
						headers: {
								'Content-Type': 'application/x-www-form-urlencoded'
						},
						body: params
				})
				.then(response => response.json())
				.then(data => {
						if (data.success) {
								delete SERVO_STATE.presets[name];
								updatePresetDisplay();
								updateServoStats();
								showAlert('success', `Preset "${name}" deleted`);
						} else {
								showAlert('danger', `Error: ${data.message}`);
						}
				})
				.catch(error => {
						console.error('Error deleting preset:', error);
						showAlert('danger', 'Error deleting preset');
				});
		}
		
		// ============ END SERVO FUNCTIONS ============
		
		function formatBytes(bytes) {
				if (bytes === 0) return '0 B';
				const k = 1024;
				const sizes = ['B', 'KB', 'MB'];
				const i = Math.floor(Math.log(bytes) / Math.log(k));
				return parseFloat((bytes / Math.pow(k, i)).toFixed(1)) + ' ' + sizes[i];
		}
		
		// Dashboard navigation
		navLinks.forEach(link => {
				link.addEventListener('click', function(e) {
						e.preventDefault();
						const hash = this.getAttribute('href');
						navigateTo(hash);
				});
		});
		
		// Logout button
		logoutBtn.addEventListener('click', logout);
		
		// Update stats every second (only for dashboard)
		setInterval(() => {
				if (APP_STATE.currentDashboardSection === 'dashboard' && dashboardWrapper.style.display !== 'none') {
						fetchSystemStats();
				}
		}, 1000);
		
		// Initialize the app
		initApp();
		
		// Store boot time for uptime calculation
		localStorage.setItem('boot_time', APP_STATE.bootTime);
		
		// Utility function to show alerts
		function showAlert(type, message) {
				// Create alert container if it doesn't exist
				let alertContainer = document.getElementById('alert-container');
				if (!alertContainer) {
						alertContainer = document.createElement('div');
						alertContainer.id = 'alert-container';
						alertContainer.style.position = 'fixed';
						alertContainer.style.top = '20px';
						alertContainer.style.right = '20px';
						alertContainer.style.zIndex = '9999';
						alertContainer.style.maxWidth = '400px';
						document.body.appendChild(alertContainer);
				}
				
				// Create alert
				const alert = document.createElement('div');
				alert.className = `alert alert-${type} alert-dismissible fade show`;
				alert.style.marginBottom = '10px';
				alert.innerHTML = `
						${message}
						<button type="button" class="btn-close" data-bs-dismiss="alert"></button>
				`;
				
				alertContainer.appendChild(alert);
				
				// Auto-dismiss after 5 seconds
				setTimeout(() => {
						if (alert.parentNode) {
								alert.remove();
						}
				}, 5000);
		}
		
		// Make servo functions globally accessible
		window.loadPreset = loadPreset;
		window.deletePreset = deletePreset;
});