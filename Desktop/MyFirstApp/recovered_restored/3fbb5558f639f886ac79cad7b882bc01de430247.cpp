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

        this.init();
    }

    init() {
        this.bindEvents();
        this.render();
        this.updateStats();
        this.requestNotificationPermission();
        this.checkOverdueTasks();
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
                this.importData(file);
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
}

// 应用初始化
document.addEventListener('DOMContentLoaded', () => {
    new TodoApp();
});