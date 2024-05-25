#include <stdio.h>
#include <stdlib.h>
#include <libudev.h>

/****
udev_new(): Создает и возвращает новый контекст udev.
udev_monitor_new_from_netlink(): Создает и возвращает новый монитор udev, связанный с заданным типом событий (в данном случае, "udev") через Netlink.
udev_monitor_filter_add_match_subsystem_devtype(): Добавляет фильтр на монитор устройств, чтобы он отслеживал только устройства с заданной подсистемой и типом устройства.
udev_monitor_enable_receiving(): Включает прием событий на мониторе устройств.
udev_enumerate_new(): Создает и возвращает новый объект перечислителя устройств udev.
udev_enumerate_add_match_subsystem(): Добавляет фильтр в объект перечислителя устройств, чтобы он перечислял только устройства с заданной подсистемой.
udev_enumerate_scan_devices(): Запускает сканирование устройств согласно установленным фильтрам в объекте перечислителя устройств.
udev_enumerate_get_list_entry(): Возвращает список устройств, обнаруженных в результате сканирования, в виде связанного списка.
udev_list_entry_foreach(): Перебирает элементы связанного списка, представленного в виде объекта списка.
*/

// Функция инициализации контекста udev
struct udev *initialize_udev() {
    struct udev *udev = udev_new();
    if (!udev) {
        fprintf(stderr, "Ошибка при создании контекста udev\n");
        exit(1);
    }
    return udev;
}

// Функция создания udev monitor и настройки фильтров
struct udev_monitor *initialize_udev_monitor(struct udev *udev) {
    struct udev_monitor *monitor = udev_monitor_new_from_netlink(udev, "udev");
    udev_monitor_filter_add_match_subsystem_devtype(monitor, "input", NULL);
    udev_monitor_enable_receiving(monitor);
    return monitor;
}

// Функция получения начального списка устройств ввода
void print_initial_input_devices(struct udev *udev) {
   if(udev)
   {
    struct udev_enumerate *enumerate = udev_enumerate_new(udev);
    udev_enumerate_add_match_subsystem(enumerate, "input");
    udev_enumerate_scan_devices(enumerate);
    struct udev_list_entry *devices = udev_enumerate_get_list_entry(enumerate);
    struct udev_list_entry *entry;
    printf("Список устройств ввода:\n");
    udev_list_entry_foreach(entry, devices) {
        const char *path = udev_list_entry_get_name(entry);
        struct udev_device *dev = udev_device_new_from_syspath(udev, path);
        printf("- Устройство: %s\n", udev_device_get_devnode(dev));
        udev_device_unref(dev);
    }
    printf("\n");
    udev_enumerate_unref(enumerate);
   }else{
     printf(stderr, "Func print_initial_input_devices :: Empty context");
   } 
}

// Функция отслеживания изменений в устройствах ввода
void monitor_input_devices(struct udev *udev, struct udev_monitor *monitor) {
    while (1) {
        if (udev_monitor_get_fd(monitor) > 0) {
            fd_set fds;
            FD_ZERO(&fds);
            FD_SET(udev_monitor_get_fd(monitor), &fds);
            int ret = select(udev_monitor_get_fd(monitor) + 1, &fds, NULL, NULL, NULL);
            if (ret > 0 && FD_ISSET(udev_monitor_get_fd(monitor), &fds)) {
                struct udev_device *dev = udev_monitor_receive_device(monitor);
                if (dev) {
                    const char *action = udev_device_get_action(dev);
                    if (action && strcmp(action, "add") == 0) {
                        printf("Добавлено устройство: %s\n", udev_device_get_devnode(dev));
                    } else if (action && strcmp(action, "remove") == 0) {
                        printf("Удалено устройство: %s\n", udev_device_get_devnode(dev));
                    }
                    udev_device_unref(dev);
                }
            }
        }
    }
}

int main() {
    struct udev *udev = initialize_udev();
    struct udev_monitor *monitor = initialize_udev_monitor(udev);
    print_initial_input_devices(udev);
    monitor_input_devices(udev, monitor);

    // Освобождение ресурсов
    udev_monitor_unref(monitor);
    udev_unref(udev);

    return 0;
}

