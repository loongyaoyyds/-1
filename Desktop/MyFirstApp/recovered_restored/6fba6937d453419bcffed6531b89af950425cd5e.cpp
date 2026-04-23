class TodoApp {
    constructor() {
        this.tasks = this.loadTasks();
        this.currentFilter = {
            priority: 'all',
            category: 'all',
            status: 'all'
        };
        this.currentSort = 'date-created';
        this.editingTaskId = null;
        this.reminderIntervals = [];

        // Cloud sync settings
        this.cloudSyncEnabled = false;
        this.supabase = null;
        this.currentUser = null;
        this.syncInProgress = false;
        this.setupSupabase();

        this.init();
    }

    init() {
        this.bindEvents();
        this.render();
        this.updateStats();
        this.requestNotificationPermission();
        this.checkOverdueTasks();

        // Initialize cloud sync after other features
        setTimeout(() => this.setupSupabase(), 1000);
    }

    bindEvents() {
        // 添加任务按钮
        document.getElementById('add-task-btn').addEventListener('click', () => {
            this.openModal();
        });

        // 模态框相关事件
        const modal = document.getElementById('task-modal');
        const closeBtn = document.querySelector('.close');
        const cancelBtn = document.getElementById('cancel-btn');
        const taskForm = document.getElementById('task-form');

        closeBtn.addEventListener('click', () => this.closeModal());
        cancelBtn.addEventListener('click', () => this.closeModal());
        modal.addEventListener('click', (e) => {
            if (e.target === modal) this.closeModal();
        });

        taskForm.addEventListener('submit', (e) => {
            e.preventDefault();
            this.saveTask();
        });

        // 筛选器事件
        document.getElementById('priority-filter').addEventListener('change', (e) => {
            this.currentFilter.priority = e.target.value;
            this.render();
        });

        document.getElementById('category-filter').addEventListener('change', (e) => {
            this.currentFilter.category = e.target.value;
            this.render();
        });

        document.getElementById('status-filter').addEventListener('change', (e) => {
            this.currentFilter.status = e.target.value;
            this.render();
        });

        // 排序事件
        document.getElementById('sort-by').addEventListener('change', (e) => {
            this.currentSort = e.target.value;
            this.render();
        });

        // Add data management buttons
        this.addDataManagementButtons();

        // Add theme toggle button
        this.addThemeToggleButton();

        // Add search functionality
        this.addSearchFunctionality();

        // Setup keyboard shortcuts
        this.setupKeyboardShortcuts();
    }

    addThemeToggleButton() {
        // Add theme toggle to header
        const header = document.querySelector('header');
        const statsDiv = header.querySelector('.stats');

        // Create theme toggle button
        const themeToggle = document.createElement('button');
        themeToggle.id = 'theme-toggle';
        themeToggle.className = 'btn-icon';
        themeToggle.title = '切换主题';
        themeToggle.innerHTML = '🌓';

        // Insert before stats
        statsDiv.parentNode.insertBefore(themeToggle, statsDiv);

        // Load saved theme
        const savedTheme = localStorage.getItem('todo-theme') || 'light';
        this.applyTheme(savedTheme);

        // Bind event
        themeToggle.addEventListener('click', () => {
            const currentTheme = localStorage.getItem('todo-theme') || 'light';
            const newTheme = currentTheme === 'light' ? 'dark' : 'light';
            this.applyTheme(newTheme);
            localStorage.setItem('todo-theme', newTheme);
        });
    }

    applyTheme(theme) {
        const body = document.body;
        if (theme === 'dark') {
            body.classList.add('dark-theme');
            this.showInAppNotification('主题已切换', '已启用深色模式', 'info');
        } else {
            body.classList.remove('dark-theme');
            this.showInAppNotification('主题已切换', '已启用浅色模式', 'info');
        }
    }

    addSearchFunctionality() {
        // Add search box to task list header
        const taskListHeader = document.querySelector('.task-list-header');
        const sortOptions = taskListHeader.querySelector('.sort-options');

        // Create search container
        const searchContainer = document.createElement('div');
        searchContainer.className = 'search-container';
        searchContainer.innerHTML = `
            <input type="text" id="search-input" placeholder="搜索任务..." class="search-input">
            <button id="clear-search" class="btn-icon" title="清除搜索">✕</button>
        `;

        // Insert before sort options
        sortOptions.parentNode.insertBefore(searchContainer, sortOptions);

        // Bind search events
        const searchInput = document.getElementById('search-input');
        const clearButton = document.getElementById('clear-search');

        searchInput.addEventListener('input', (e) => {
            this.handleSearch(e.target.value);
        });

        clearButton.addEventListener('click', () => {
            searchInput.value = '';
            this.handleSearch('');
        });
    }

    handleSearch(query) {
        this.currentSearch = query.toLowerCase().trim();
        this.render();
    }

    addIntelligentFeatures() {
        // Add statistics dashboard
        this.addStatisticsDashboard();

        // Add achievement system
        this.addAchievementSystem();

        // Setup smart categorization
        this.setupSmartCategorization();
    }

    addStatisticsDashboard() {
        // Add stats button to header
        const header = document.querySelector('header');
        const statsDiv = header.querySelector('.stats');

        // Create view stats button
        const statsButton = document.createElement('button');
        statsButton.id = 'view-stats';
        statsButton.className = 'btn-icon';
        statsButton.title = '查看统计';
        statsButton.innerHTML = '📊';

        // Insert before theme toggle
        const themeToggle = document.getElementById('theme-toggle');
        if (themeToggle) {
            themeToggle.parentNode.insertBefore(statsButton, themeToggle);
        } else {
            statsDiv.parentNode.insertBefore(statsButton, statsDiv);
        }

        // Create stats modal
        const statsModal = document.createElement('div');
        statsModal.id = 'stats-modal';
        statsModal.className = 'modal';
        statsModal.innerHTML = `
            <div class="modal-content">
                <span class="close">&times;</span>
                <h2>📊 任务统计</h2>
                <div id="stats-content">
                    <div class="stats-grid">
                        <div class="stat-card" id="total-tasks-card">
                            <h3>总任务数</h3>
                            <div class="stat-value">0</div>
                        </div>
                        <div class="stat-card" id="completed-tasks-card">
                            <h3>已完成</h3>
                            <div class="stat-value">0</div>
                        </div>
                        <div class="stat-card" id="pending-tasks-card">
                            <h3>待完成</h3>
                            <div class="stat-value">0</div>
                        </div>
                        <div class="stat-card" id="completion-rate-card">
                            <h3>完成率</h3>
                            <div class="stat-value">0%</div>
                        </div>
                    </div>
                    <div class="stats-charts">
                        <h3>优先级分布</h3>
                        <div id="priority-chart"></div>
                        <h3>分类分布</h3>
                        <div id="category-chart"></div>
                    </div>
                </div>
            </div>
        `;

        document.body.appendChild(statsModal);

        // Bind events
        statsButton.addEventListener('click', () => {
            this.updateStatistics();
            statsModal.style.display = 'block';
        });

        const closeBtn = statsModal.querySelector('.close');
        closeBtn.addEventListener('click', () => {
            statsModal.style.display = 'none';
        });

        statsModal.addEventListener('click', (e) => {
            if (e.target === statsModal) {
                statsModal.style.display = 'none';
            }
        });
    }

    updateStatistics() {
        const total = this.tasks.length;
        const completed = this.tasks.filter(t => t.completed).length;
        const pending = total - completed;
        const completionRate = total > 0 ? Math.round((completed / total) * 100) : 0;

        // Update basic stats
        document.getElementById('total-tasks-card').querySelector('.stat-value').textContent = total;
        document.getElementById('completed-tasks-card').querySelector('.stat-value').textContent = completed;
        document.getElementById('pending-tasks-card').querySelector('.stat-value').textContent = pending;
        document.getElementById('completion-rate-card').querySelector('.stat-value').textContent = completionRate + '%';

        // Update priority distribution
        const priorityStats = { high: 0, medium: 0, low: 0 };
        this.tasks.forEach(task => {
            priorityStats[task.priority]++;
        });
        this.updateChart('priority-chart', priorityStats, ['高优先级', '中优先级', '低优先级']);

        // Update category distribution
        const categoryStats = {};
        this.tasks.forEach(task => {
            if (task.category) {
                categoryStats[task.category] = (categoryStats[task.category] || 0) + 1;
            }
        });
        const sortedCategories = Object.entries(categoryStats)
            .sort(([,a], [,b]) => b - a)
            .slice(0, 5); // Top 5 categories
        this.updateChart('category-chart', Object.fromEntries(sortedCategories), sortedCategories.map(([name]) => name));
    }

    updateChart(containerId, data, labels) {
        const container = document.getElementById(containerId);
        const total = Object.values(data).reduce((a, b) => a + b, 0);

        container.innerHTML = `
            <div class="chart-bars">
                ${labels.map((label, index) => {
                    const value = data[Object.keys(data)[index]] || 0;
                    const percentage = total > 0 ? (value / total * 100) : 0;
                    return `
                        <div class="chart-bar-item">
                            <div class="bar-label">${label}</div>
                            <div class="bar-container">
                                <div class="bar" style="width: ${percentage}%"></div>
                                <span class="bar-value">${value}</span>
                            </div>
                        </div>
                    `;
                }).join('')}
            </div>
        `;
    }

    addAchievementSystem() {
        // Create achievement modal
        const achievementModal = document.createElement('div');
        achievementModal.id = 'achievement-modal';
        achievementModal.className = 'modal';
        achievementModal.innerHTML = `
            <div class="modal-content">
                <span class="close">&times;</span>
                <h2>🏆 成就系统</h2>
                <div id="achievement-content">
                    <div class="achievement-grid" id="achievement-grid">
                        <!-- Achievements will be populated here -->
                    </div>
                </div>
            </div>
        `;

        document.body.appendChild(achievementModal);

        // Add achievement button
        const header = document.querySelector('header');
        const statsDiv = header.querySelector('.stats');

        const achievementButton = document.createElement('button');
        achievementButton.id = 'view-achievements';
        achievementButton.className = 'btn-icon';
        achievementButton.title = '查看成就';
        achievementButton.innerHTML = '🏆';

        const viewStats = document.getElementById('view-stats');
        if (viewStats) {
            viewStats.parentNode.insertBefore(achievementButton, viewStats);
        }

        // Bind events
        achievementButton.addEventListener('click', () => {
            this.updateAchievements();
            achievementModal.style.display = 'block';
        });

        const closeBtn = achievementModal.querySelector('.close');
        closeBtn.addEventListener('click', () => {
            achievementModal.style.display = 'none';
        });

        achievementModal.addEventListener('click', (e) => {
            if (e.target === achievementModal) {
                achievementModal.style.display = 'none';
            }
        });
    }

    updateAchievements() {
        const achievements = this.calculateAchievements();
        const grid = document.getElementById('achievement-grid');

        const achievementList = [
            { id: 'first-task', name: '初来乍到', desc: '创建第一个任务', icon: '🎯', check: () => this.tasks.length >= 1 },
            { id: 'ten-tasks', name: '任务达人', desc: '创建10个任务', icon: '📝', check: () => this.tasks.length >= 10 },
            { id: 'complete-five', name: '完成者', desc: '完成5个任务', icon: '✅', check: () => this.tasks.filter(t => t.completed).length >= 5 },
            { id: 'perfect-week', name: '完美一周', desc: '一周内完成所有任务', icon: '🌟', check: () => this.checkPerfectWeek() },
            { id: 'categorize-all', name: '分类达人', desc: '为所有任务添加分类', icon: '🏷️', check: () => this.tasks.length > 0 && this.tasks.every(t => t.category) },
            { id: 'priority-master', name: '优先级大师', desc: '创建高优先级任务', icon: '🔴', check: () => this.tasks.some(t => t.priority === 'high') },
            { id: 'early-bird', name: '早起鸟', desc: '创建今日截止任务', icon: '🌅', check: () => this.tasks.some(t => t.dueDate && new Date(t.dueDate).toISOString().split('T')[0] === new Date().toISOString().split('T')[0]) },
            { id: 'streak-3', name: '连续完成', desc: '连续3天完成任务', icon: '🔥', check: () => this.checkStreak(3) }
        ];

        grid.innerHTML = achievementList.map(achievement => {
            const isUnlocked = achievement.check();
            return `
                <div class="achievement-item ${isUnlocked ? 'unlocked' : 'locked'}" data-achievement="${achievement.id}">
                    <div class="achievement-icon">${achievement.icon}</div>
                    <div class="achievement-info">
                        <h4>${achievement.name}</h4>
                        <p>${achievement.desc}</p>
                    </div>
                    <div class="achievement-status">
                        ${isUnlocked ? '✅' : '🔒'}
                    </div>
                </div>
            `;
        }).join('');

        // Show notification for newly unlocked achievements
        achievementList.forEach(achievement => {
            if (achievement.check() && !localStorage.getItem(`achievement-${achievement.id}`)) {
                this.showInAppNotification('🏆 新成就解锁！', `恭喜获得成就: ${achievement.name}`, 'success');
                localStorage.setItem(`achievement-${achievement.id}`, 'true');
            }
        });
    }

    checkPerfectWeek() {
        // Check if all tasks created in the last week are completed
        const oneWeekAgo = new Date();
        oneWeekAgo.setDate(oneWeekAgo.getDate() - 7);

        const recentTasks = this.tasks.filter(t => new Date(t.createdAt) >= oneWeekAgo);
        return recentTasks.length > 0 && recentTasks.every(t => t.completed);
    }

    checkStreak(days) {
        // Simple streak check - in a real app, you'd want more sophisticated tracking
        const completedDates = [...new Set(this.tasks.filter(t => t.completed).map(t => new Date(t.updatedAt).toISOString().split('T')[0]))];
        return completedDates.length >= days;
    }

    setupSmartCategorization() {
        // Add auto-categorization suggestions based on task title keywords
        const taskTitleInput = document.getElementById('task-title');
        if (taskTitleInput) {
            taskTitleInput.addEventListener('blur', (e) => {
                const title = e.target.value.toLowerCase();
                const categoryInput = document.getElementById('task-category');

                // Only suggest if category is empty
                if (categoryInput && !categoryInput.value) {
                    let suggestedCategory = '';

                    // Keyword matching for categories
                    const keywords = {
                        '工作': ['工作', '上班', '会议', '项目', '任务', '报告'],
                        '学习': ['学习', '读书', '看书', '课程', '考试', '作业', '学习'],
                        '生活': ['生活', '购物', '买菜', '做饭', '洗衣', '打扫'],
                        '健康': ['运动', '健身', '跑步', '锻炼', '健康', '看病'],
                        '娱乐': ['娱乐', '电影', '音乐', '游戏', '旅游', '聚会']
                    };

                    for (const [category, words] of Object.entries(keywords)) {
                        if (words.some(word => title.includes(word))) {
                            suggestedCategory = category;
                            break;
                        }
                    }

                    if (suggestedCategory) {
                        categoryInput.value = suggestedCategory;
                        this.showInAppNotification('💡 智能分类', `已根据任务标题建议分类: ${suggestedCategory}`, 'info');
                    }
                }
            });
        }
    }

    setupKeyboardShortcuts() {
        // Global keyboard shortcuts
        document.addEventListener('keydown', (e) => {
            // Ctrl/Cmd + N: Add new task
            if ((e.ctrlKey || e.metaKey) && e.key === 'n') {
                e.preventDefault();
                this.openModal();
            }

            // Ctrl/Cmd + F: Focus search
            if ((e.ctrlKey || e.metaKey) && e.key === 'f') {
                e.preventDefault();
                const searchInput = document.getElementById('search-input');
                if (searchInput) {
                    searchInput.focus();
                }
            }

            // Escape: Close modal or clear search
            if (e.key === 'Escape') {
                const modal = document.getElementById('task-modal');
                if (modal.style.display === 'block') {
                    this.closeModal();
                } else {
                    const searchInput = document.getElementById('search-input');
                    if (searchInput && searchInput.value) {
                        searchInput.value = '';
                        this.handleSearch('');
                    }
                }
            }

            // Ctrl/Cmd + S: Save task (when modal is open)
            if ((e.ctrlKey || e.metaKey) && e.key === 's') {
                e.preventDefault();
                const modal = document.getElementById('task-modal');
                if (modal.style.display === 'block') {
                    this.saveTask();
                }
            }
        });

        // Modal-specific shortcuts
        const taskForm = document.getElementById('task-form');
        if (taskForm) {
            taskForm.addEventListener('keydown', (e) => {
                // Enter in input fields should not submit form
                if (e.key === 'Enter' && e.target.tagName !== 'TEXTAREA') {
                    e.preventDefault();
                }
            });
        }
    }

    addDataManagementButtons() {
        // Add export/import buttons to the sidebar
        const sidebar = document.querySelector('.sidebar');
        const existingSection = sidebar.querySelector('.add-task-section');

        // Create data management section
        const dataSection = document.createElement('div');
        dataSection.className = 'data-management-section';
        dataSection.innerHTML = `
            <h3>数据管理</h3>
            <div class="data-actions">
                <button id="export-json" class="btn-data">导出 JSON</button>
                <button id="export-csv" class="btn-data">导出 CSV</button>
                <button id="import-data" class="btn-data">导入数据</button>
                <button id="backup-data" class="btn-data">备份数据</button>
                <button id="restore-data" class="btn-data">恢复数据</button>
            </div>
            <input type="file" id="file-input" style="display: none;" accept=".json,.csv">
        `;

        // Insert after add task section
        existingSection.parentNode.insertBefore(dataSection, existingSection.nextSibling);

        // Bind events
        document.getElementById('export-json').addEventListener('click', () => this.exportToJSON());
        document.getElementById('export-csv').addEventListener('click', () => this.exportToCSV());
        document.getElementById('import-data').addEventListener('click', () => {
            document.getElementById('file-input').click();
        });
        document.getElementById('backup-data').addEventListener('click', () => this.backupData());
        document.getElementById('restore-data').addEventListener('click', () => {
            if (confirm('确定要恢复备份吗？这将覆盖当前所有数据。')) {
                document.getElementById('file-input').click();
            }
        });

        // File input change event
        document.getElementById('file-input').addEventListener('change', (e) => {
            const file = e.target.files[0];
            if (file) {
                // Check if it's a backup file
                if (file.name.includes('backup')) {
                    this.restoreData(file);
                } else {
                    this.importData(file);
                }
            }
            // Reset file input
            e.target.value = '';
        });
    }

    loadTasks() {
        const saved = localStorage.getItem('todo-tasks');
        return saved ? JSON.parse(saved) : [];
    }

    saveTasks() {
        localStorage.setItem('todo-tasks', JSON.stringify(this.tasks));
    }

    generateId() {
        return Date.now().toString(36) + Math.random().toString(36).substr(2);
    }

    openModal(taskId = null) {
        const modal = document.getElementById('task-modal');
        const modalTitle = document.getElementById('modal-title');

        if (taskId) {
            modalTitle.textContent = '编辑任务';
            this.editingTaskId = taskId;
            this.populateForm(taskId);
        } else {
            modalTitle.textContent = '添加新任务';
            this.editingTaskId = null;
            document.getElementById('task-form').reset();
        }

        modal.style.display = 'block';
    }

    closeModal() {
        document.getElementById('task-modal').style.display = 'none';
    }

    populateForm(taskId) {
        const task = this.tasks.find(t => t.id === taskId);
        if (task) {
            document.getElementById('task-title').value = task.title;
            document.getElementById('task-description').value = task.description || '';
            document.getElementById('task-priority').value = task.priority;
            document.getElementById('task-category').value = task.category || '';
            document.getElementById('task-due-date').value = task.dueDate || '';
        }
    }

    saveTask() {
        const formData = {
            title: document.getElementById('task-title').value,
            description: document.getElementById('task-description').value,
            priority: document.getElementById('task-priority').value,
            category: document.getElementById('task-category').value,
            dueDate: document.getElementById('task-due-date').value
        };

        if (!formData.title.trim()) {
            alert('请输入任务标题');
            return;
        }

        if (this.editingTaskId) {
            // 编辑现有任务
            const task = this.tasks.find(t => t.id === this.editingTaskId);
            if (task) {
                Object.assign(task, formData);
                task.updatedAt = new Date().toISOString();
            }
        } else {
            // 添加新任务
            const newTask = {
                id: this.generateId(),
                ...formData,
                completed: false,
                createdAt: new Date().toISOString(),
                updatedAt: new Date().toISOString()
            };
            this.tasks.push(newTask);
        }

        this.saveTasks();
        this.closeModal();
        this.render();
        this.updateStats();
        this.setupTaskReminders(formData);
    }

    toggleTask(taskId) {
        const task = this.tasks.find(t => t.id === taskId);
        if (task) {
            task.completed = !task.completed;
            task.updatedAt = new Date().toISOString();
            this.saveTasks();
            this.render();
            this.updateStats();

            // Clear reminders if task is completed
            if (task.completed) {
                this.clearTaskReminders(taskId);
            } else {
                // Re-setup reminders if task is re-opened
                this.setupTaskReminders(task);
            }
        }
    }

    editTask(taskId) {
        this.openModal(taskId);
    }

    deleteTask(taskId) {
        if (confirm('确定要删除这个任务吗？')) {
            this.tasks = this.tasks.filter(t => t.id !== taskId);
            this.saveTasks();
            this.render();
            this.updateStats();
        }
    }

    getFilteredAndSortedTasks() {
        let filtered = [...this.tasks];

        // Apply search filter
        if (this.currentSearch) {
            filtered = filtered.filter(task => {
                const searchFields = [
                    task.title,
                    task.description,
                    task.category
                ].filter(Boolean);
                return searchFields.some(field =>
                    field.toLowerCase().includes(this.currentSearch)
                );
            });
        }

        // 应用筛选
        if (this.currentFilter.priority !== 'all') {
            filtered = filtered.filter(t => t.priority === this.currentFilter.priority);
        }

        if (this.currentFilter.category !== 'all') {
            filtered = filtered.filter(t => t.category === this.currentFilter.category);
        }

        if (this.currentFilter.status !== 'all') {
            filtered = filtered.filter(t => {
                if (this.currentFilter.status === 'completed') return t.completed;
                if (this.currentFilter.status === 'pending') return !t.completed;
                return true;
            });
        }

        // 应用排序
        filtered.sort((a, b) => {
            switch (this.currentSort) {
                case 'date-created':
                    return new Date(b.createdAt) - new Date(a.createdAt);
                case 'due-date':
                    if (!a.dueDate) return 1;
                    if (!b.dueDate) return -1;
                    return new Date(a.dueDate) - new Date(b.dueDate);
                case 'priority':
                    const priorityOrder = { high: 3, medium: 2, low: 1 };
                    return priorityOrder[b.priority] - priorityOrder[a.priority];
                default:
                    return 0;
            }
        });

        return filtered;
    }

    render() {
        const taskList = document.getElementById('task-list');
        const filteredTasks = this.getFilteredAndSortedTasks();

        // 更新分类筛选器
        this.updateCategoryFilter();

        if (filteredTasks.length === 0) {
            taskList.innerHTML = `
                <div class="empty-state">
                    <div class="empty-state-icon">📝</div>
                    <h3>暂无任务</h3>
                    <p>点击"添加新任务"开始创建你的第一个任务吧！</p>
                </div>
            `;
            return;
        }

        taskList.innerHTML = filteredTasks.map(task => this.renderTask(task)).join('');

        // 绑定任务操作事件
        filteredTasks.forEach(task => {
            const taskElement = document.querySelector(`[data-task-id="${task.id}"]`);
            if (taskElement) {
                taskElement.querySelector('.task-checkbox').addEventListener('change', () => {
                    this.toggleTask(task.id);
                });

                taskElement.querySelector('.btn-edit').addEventListener('click', () => {
                    this.editTask(task.id);
                });

                taskElement.querySelector('.btn-delete').addEventListener('click', () => {
                    this.deleteTask(task.id);
                });
            }
        });
    }

    renderTask(task) {
        const priorityClass = `priority-${task.priority}`;
        const isOverdue = task.dueDate && !task.completed && new Date(task.dueDate) < new Date();
        const dueDateClass = isOverdue ? 'overdue' : '';
        const dueDateText = task.dueDate ? new Date(task.dueDate).toLocaleDateString('zh-CN') : '未设置';

        return `
            <div class="task-item ${task.completed ? 'completed' : ''}" data-task-id="${task.id}">
                <input type="checkbox" class="task-checkbox" ${task.completed ? 'checked' : ''}>
                <div class="task-priority ${priorityClass}"></div>
                <div class="task-content">
                    <h4>${this.escapeHtml(task.title)}</h4>
                    ${task.description ? `<p class="task-description">${this.escapeHtml(task.description)}</p>` : ''}
                    <div class="task-meta">
                        <span class="task-category">${task.category ? this.escapeHtml(task.category) : '未分类'}</span>
                        <span class="task-due-date ${dueDateClass}" title="${isOverdue ? '已过期' : '截止日期'}">
                            ${isOverdue ? '⚠️ ' : '📅 '}${dueDateText}
                        </span>
                    </div>
                </div>
                <div class="task-actions">
                    <button class="btn-icon btn-edit" title="编辑">✏️</button>
                    <button class="btn-icon btn-delete" title="删除">🗑️</button>
                </div>
            </div>
        `;
    }

    updateCategoryFilter() {
        const categoryFilter = document.getElementById('category-filter');
        const categories = [...new Set(this.tasks.map(t => t.category).filter(c => c))].sort();

        // 保存当前选中的分类
        const currentCategory = this.currentFilter.category;

        // 重新生成分类选项
        const categoryOptions = ['<option value="all">全部分类</option>']
            .concat(categories.map(cat => `<option value="${this.escapeHtml(cat)}">${this.escapeHtml(cat)}</option>`))
            .join('');

        categoryFilter.innerHTML = categoryOptions;

        // 恢复之前选中的分类
        if (categories.includes(currentCategory) || currentCategory === 'all') {
            categoryFilter.value = currentCategory;
        }
    }

    updateStats() {
        const total = this.tasks.length;
        const completed = this.tasks.filter(t => t.completed).length;

        document.getElementById('total-tasks').textContent = `${total} 个任务`;
        document.getElementById('completed-tasks').textContent = `${completed} 已完成`;
    }

    escapeHtml(text) {
        const div = document.createElement('div');
        div.textContent = text;
        return div.innerHTML;
    }

    requestNotificationPermission() {
        // Request notification permission if supported
        if ('Notification' in window) {
            Notification.requestPermission().then(permission => {
                console.log('Notification permission:', permission);
            });
        }
    }

    checkOverdueTasks() {
        // Check for overdue tasks and send notifications
        const now = new Date();
        const today = now.toISOString().split('T')[0];

        this.tasks.forEach(task => {
            if (task.dueDate && !task.completed) {
                const dueDate = new Date(task.dueDate);
                const todayDate = new Date(today);

                // Check if task is overdue
                if (dueDate < todayDate) {
                    this.sendNotification(
                        '任务已过期',
                        `任务 "${task.title}" 已于 ${dueDate.toLocaleDateString('zh-CN')} 到期`,
                        'overdue'
                    );
                }
                // Check if task is due today
                else if (dueDate.toISOString().split('T')[0] === today) {
                    this.sendNotification(
                        '任务今天到期',
                        `任务 "${task.title}" 今天到期`,
                        'today'
                    );
                }
                // Check if task is due tomorrow
                else {
                    const tomorrow = new Date(today);
                    tomorrow.setDate(tomorrow.getDate() + 1);
                    if (dueDate.toISOString().split('T')[0] === tomorrow.toISOString().split('T')[0]) {
                        this.sendNotification(
                            '任务明天到期',
                            `任务 "${task.title}" 明天到期`,
                            'tomorrow'
                        );
                    }
                }
            }
        });
    }

    sendNotification(title, body, type = 'info') {
        // Send desktop notification if permission granted
        if ('Notification' in window && Notification.permission === 'granted') {
            const notification = new Notification(title, {
                body: body,
                icon: '📝',
                tag: `todo-${type}`
            });

            notification.onclick = () => {
                window.focus();
                notification.close();
            };

            // Auto close after 5 seconds
            setTimeout(() => notification.close(), 5000);
        }

        // Also show in-app notification
        this.showInAppNotification(title, body, type);
    }

    showInAppNotification(title, message, type = 'info') {
        // Create notification element
        const notification = document.createElement('div');
        notification.className = `notification notification-${type}`;
        notification.innerHTML = `
            <div class="notification-content">
                <strong>${this.escapeHtml(title)}</strong>
                <p>${this.escapeHtml(message)}</p>
            </div>
            <button class="notification-close">&times;</button>
        `;

        // Add to page
        document.body.appendChild(notification);

        // Auto remove after 5 seconds
        setTimeout(() => {
            if (notification.parentNode) {
                notification.remove();
            }
        }, 5000);

        // Close button event
        notification.querySelector('.notification-close').addEventListener('click', () => {
            notification.remove();
        });
    }

    exportToJSON() {
        const dataStr = JSON.stringify(this.tasks, null, 2);
        const blob = new Blob([dataStr], { type: 'application/json' });
        const url = URL.createObjectURL(blob);
        const a = document.createElement('a');
        a.href = url;
        a.download = `todo-tasks-${new Date().toISOString().split('T')[0]}.json`;
        document.body.appendChild(a);
        a.click();
        document.body.removeChild(a);
        URL.revokeObjectURL(url);

        this.showInAppNotification('导出成功', '任务数据已导出为 JSON 文件', 'success');
    }

    exportToCSV() {
        if (this.tasks.length === 0) {
            this.showInAppNotification('导出失败', '没有任务数据可导出', 'warning');
            return;
        }

        const headers = ['ID', '标题', '描述', '优先级', '分类', '截止日期', '完成状态', '创建时间', '更新时间'];
        const csvContent = [
            headers.join(','),
            ...this.tasks.map(task => [
                `"${task.id}"`,
                `"${task.title.replace(/"/g, '""')}"`,
                `"${(task.description || '').replace(/"/g, '""')}"`,
                `"${task.priority}"`,
                `"${task.category || ''}"`,
                `"${task.dueDate || ''}"`,
                `"${task.completed ? '已完成' : '待完成'}"`,
                `"${task.createdAt}"`,
                `"${task.updatedAt}"`
            ].join(','))
        ].join('\n');

        const blob = new Blob(['\ufeff' + csvContent], { type: 'text/csv;charset=utf-8;' });
        const url = URL.createObjectURL(blob);
        const a = document.createElement('a');
        a.href = url;
        a.download = `todo-tasks-${new Date().toISOString().split('T')[0]}.csv`;
        document.body.appendChild(a);
        a.click();
        document.body.removeChild(a);
        URL.revokeObjectURL(url);

        this.showInAppNotification('导出成功', '任务数据已导出为 CSV 文件', 'success');
    }

    importData(file) {
        const reader = new FileReader();
        reader.onload = (e) => {
            try {
                const content = e.target.result;
                let importedTasks;

                if (file.name.endsWith('.json')) {
                    importedTasks = JSON.parse(content);
                } else if (file.name.endsWith('.csv')) {
                    importedTasks = this.parseCSV(content);
                } else {
                    throw new Error('不支持的文件格式');
                }

                // Validate imported data
                if (!Array.isArray(importedTasks)) {
                    throw new Error('导入的数据格式不正确');
                }

                // Merge with existing tasks, avoiding duplicates
                const existingIds = new Set(this.tasks.map(t => t.id));
                let importedCount = 0;
                importedTasks.forEach(task => {
                    if (!existingIds.has(task.id)) {
                        // Ensure task has required fields
                        const validatedTask = {
                            id: task.id || this.generateId(),
                            title: task.title || '未命名任务',
                            description: task.description || '',
                            priority: task.priority || 'medium',
                            category: task.category || '',
                            dueDate: task.dueDate || '',
                            completed: task.completed || false,
                            createdAt: task.createdAt || new Date().toISOString(),
                            updatedAt: task.updatedAt || new Date().toISOString()
                        };
                        this.tasks.push(validatedTask);
                        importedCount++;
                    }
                });

                this.saveTasks();
                this.render();
                this.updateStats();
                this.showInAppNotification('导入成功', `成功导入 ${importedCount} 个任务`, 'success');

                // Setup reminders for imported tasks
                this.tasks.forEach(task => {
                    if (!task.completed && task.dueDate) {
                        this.setupTaskReminders(task);
                    }
                });

            } catch (error) {
                this.showInAppNotification('导入失败', `错误: ${error.message}`, 'error');
            }
        };
        reader.readAsText(file);
    }

    parseCSV(csvContent) {
        const lines = csvContent.split('\n').filter(line => line.trim());
        if (lines.length < 2) return [];

        // Skip header and BOM if present
        const startIdx = lines[0].charCodeAt(0) === 65279 ? 1 : 0;
        const headers = lines[startIdx].split(',').map(h => h.replace(/"/g, '').trim());
        const tasks = [];

        for (let i = startIdx + 1; i < lines.length; i++) {
            const values = this.parseCSVLine(lines[i]);
            if (values.length >= headers.length) {
                const task = {};
                headers.forEach((header, index) => {
                    let value = values[index] ? values[index].replace(/^"|"$/g, '') : '';
                    // Unescape double quotes
                    value = value.replace(/""/g, '"');

                    switch (header) {
                        case 'ID':
                            task.id = value;
                            break;
                        case '标题':
                            task.title = value;
                            break;
                        case '描述':
                            task.description = value;
                            break;
                        case '优先级':
                            task.priority = value.toLowerCase() || 'medium';
                            break;
                        case '分类':
                            task.category = value;
                            break;
                        case '截止日期':
                            task.dueDate = value;
                            break;
                        case '完成状态':
                            task.completed = value === '已完成';
                            break;
                        case '创建时间':
                            task.createdAt = value || new Date().toISOString();
                            break;
                        case '更新时间':
                            task.updatedAt = value || new Date().toISOString();
                            break;
                    }
                });
                tasks.push(task);
            }
        }

        return tasks;
    }

    parseCSVLine(line) {
        const values = [];
        let current = '';
        let inQuotes = false;

        for (let i = 0; i < line.length; i++) {
            const char = line[i];

            if (char === '"') {
                if (inQuotes && i + 1 < line.length && line[i + 1] === '"') {
                    // Double quote inside quotes
                    current += '"';
                    i++; // Skip next quote
                } else {
                    inQuotes = !inQuotes;
                }
            } else if (char === ',' && !inQuotes) {
                values.push(current.trim());
                current = '';
            } else {
                current += char;
            }
        }

        values.push(current.trim());
        return values;
    }

    backupData() {
        const backup = {
            tasks: this.tasks,
            backupDate: new Date().toISOString(),
            version: '1.0'
        };

        const dataStr = JSON.stringify(backup, null, 2);
        const blob = new Blob([dataStr], { type: 'application/json' });
        const url = URL.createObjectURL(blob);
        const a = document.createElement('a');
        a.href = url;
        a.download = `todo-backup-${new Date().toISOString().split('T')[0]}.json`;
        document.body.appendChild(a);
        a.click();
        document.body.removeChild(a);
        URL.revokeObjectURL(url);

        this.showInAppNotification('备份成功', '数据已备份到本地文件', 'success');
    }

    restoreData(file) {
        const reader = new FileReader();
        reader.onload = (e) => {
            try {
                const content = e.target.result;
                let backup;

                backup = JSON.parse(content);
                if (!backup.tasks || !Array.isArray(backup.tasks)) {
                    throw new Error('备份文件格式不正确');
                }
                this.tasks = backup.tasks;

                this.saveTasks();
                this.render();
                this.updateStats();
                this.showInAppNotification('恢复成功', '数据已从备份恢复', 'success');

            } catch (error) {
                this.showInAppNotification('恢复失败', `错误: ${error.message}`, 'error');
            }
        };
        reader.readAsText(file);
    }

    setupTaskReminders(task) {
        // Clear existing reminders for this task
        this.clearTaskReminders(task.id);

        // Don't set reminders if no due date or already completed
        if (!task.dueDate || task.completed) {
            return;
        }

        const dueDate = new Date(task.dueDate);
        const now = new Date();
        const timeUntilDue = dueDate - now;

        // Only set reminders if due date is in the future
        if (timeUntilDue <= 0) {
            return;
        }

        // Set reminder at 1 day before
        const oneDayBefore = timeUntilDue - (24 * 60 * 60 * 1000);
        if (oneDayBefore > 0) {
            const reminderId = setTimeout(() => {
                this.sendNotification(
                    '任务即将到期',
                    `任务 "${task.title}" 将于明天到期`,
                    'reminder'
                );
            }, oneDayBefore);
            this.reminderIntervals.push({ taskId: task.id, intervalId: reminderId });
        }

        // Set reminder at 1 hour before
        const oneHourBefore = timeUntilDue - (60 * 60 * 1000);
        if (oneHourBefore > 0) {
            const reminderId = setTimeout(() => {
                this.sendNotification(
                    '任务即将到期',
                    `任务 "${task.title}" 将于1小时后到期`,
                    'reminder'
                );
            }, oneHourBefore);
            this.reminderIntervals.push({ taskId: task.id, intervalId: reminderId });
        }

        // Set reminder at due time
        const reminderId = setTimeout(() => {
            this.sendNotification(
                '任务已到期',
                `任务 "${task.title}" 现在已到期`,
                'overdue'
            );
        }, timeUntilDue);
        this.reminderIntervals.push({ taskId: task.id, intervalId: reminderId });
    }

    clearTaskReminders(taskId) {
        // Clear all reminders for a specific task
        this.reminderIntervals = this.reminderIntervals.filter(reminder => {
            if (reminder.taskId === taskId) {
                clearTimeout(reminder.intervalId);
                return false;
            }
            return true;
        });
    }
}

// 应用初始化
document.addEventListener('DOMContentLoaded', () => {
    new TodoApp();
});