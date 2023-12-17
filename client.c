#include <gtk/gtk.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>

#define PORT 8080

GtkBuilder *builder;
int socket_fd;

// Callback function when the window is closed
void on_main_window_destroy() {
    close(socket_fd); // Close the socket when the window is closed
    gtk_main_quit();
}

// Callback function when the button is clicked
void on_button_clicked() {
    GtkLabel *label = GTK_LABEL(gtk_builder_get_object(builder, "label1"));
    gtk_label_set_text(label, "Hello, GPT-3.5!");

    // Your code to send a message to the server
    char buffer[1024];
    sprintf(buffer, "Hello from GTK client!");
    ssize_t amount_sent = send(socket_fd, buffer, strlen(buffer), 0);
    if (amount_sent < 0) {
        perror("Error sending message to the server");
    }
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    // Create a new GtkBuilder instance
    builder = gtk_builder_new();

    // Load the UI definition from the Glade file
    gtk_builder_add_from_file(builder, "gui.glade", NULL);

    // Connect signals to callback functions
    gtk_builder_connect_signals(builder, NULL);

    // Get the main window and show it
    GtkWidget *main_window = GTK_WIDGET(gtk_builder_get_object(builder, "main_window"));
    gtk_widget_show_all(main_window);

    // Your code to create a socket and connect to the server
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_fd < 0) {
        perror("Socket creation error on client side\n");
        return -1;
    }

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    address.sin_addr.s_addr = inet_addr("127.0.0.1"); // Replace with your server's IP address

    int connect_result = connect(socket_fd, (struct sockaddr*)&address, sizeof(address));

    if (connect_result != 0) {
        perror("Connection failed.\n");
        return -1;
    }

    // Start the GTK main loop
    gtk_main();

    // Clean up resources
    close(socket_fd);
    g_object_unref(builder);

    return 0;
}

