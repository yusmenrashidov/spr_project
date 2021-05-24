#include <stdio.h>
#include <sqlite3.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <pthread.h>
#include <stdlib.h>

struct thread_struct{
    int client_socket;
    sqlite3 *db;
    char * errMsg;
};
typedef struct thread_struct thread_struct;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

int select_daily_checkout_callback(int*, int, char **, char**);
int select_tables_callback(int*, int, char **, char**);
void * handleRequest(thread_struct *);
void handleCheckFreeTables(int, sqlite3*, char*);
void handleDailyCheckout(int, sqlite3*, char*);
void handleCheckoutClientRequest(int ,sqlite3*, char*);


int main(void)
{

    int socket_desc, client_sock, client_size;
    struct sockaddr_in server_addr, client_addr;

    sqlite3 *db;
    char *err_msg = 0;
    if(sqlite3_open("/home/yusmen/spr_course_project/restaurant", &db)!=SQLITE_OK){
        printf("\nDB connection failed!!!");
        return -1;
    }
    printf("Database connection obtained successfully\n");

    // Create socket:
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);

    if(socket_desc < 0){
        printf("Error while creating socket\n");
        return -1;
    }
    printf("Socket created successfully\n");


    // Set port and IP:
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(2001);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Bind to the set port and IP:
    if(bind(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr))<0){
        printf("Couldn't bind to the port\n");
        return -1;
    }
    printf("Done with binding\n");


    // Listen for clients:
    if(listen(socket_desc, 10) < 0){
        printf("Error while listening\n");
        return -1;
    }

    //create threads
    pthread_t tid[60];
    int i=0;

    while(1){

    printf("\nListening for incoming connections.....\n");

    // Accept an incoming connection:
    client_size = sizeof(client_addr);
    client_sock = accept(socket_desc, (struct sockaddr*)&client_addr, &client_size);

    if (client_sock < 0){
        printf("Can't accept\n");
         sqlite3_close(db);
    (void) close(client_sock);
    (void) close(socket_desc);
        return -1;
    }

    thread_struct *str = malloc(sizeof(thread_struct));
    str->client_socket = client_sock;
    str->db = db;
    str->errMsg = err_msg;


       if(pthread_create(&tid[i], NULL, handleRequest, (void *) str) !=0){
        printf("Failed to process requests\n");
        free(str);
       }
         if( i >= 10)
        {
          i = 0;
          while(i < 10)
          {
            pthread_join(tid[i++],NULL);
          }
          i = 0;
        }

    }
    // Closing the socket and db_connection:
    sqlite3_close(db);
    (void) close(client_sock);
    (void) close(socket_desc);

    return 0;
}

int select_tables_callback(int *socket, int argc, char **argv, char **azColName) {

     char temp[2000];
        printf("\nDB response: %s = %s\n", azColName[0], argv[0] ? argv[0] : "NULL");

        strcpy(temp, "table number: ");
        strncat(temp, argv[0], sizeof(argv[0]));
        strncat(temp, " places: ", 10);
        strncat(temp, argv[1], sizeof(argv[1]));
        strncat(temp, " deposit: ", 10);
        strncat(temp, argv[2], sizeof(argv[2]));
        strncat(temp, "\n", 1);
        send(socket, temp, strlen(temp), 0);
        usleep(1000);
        memset(temp, '\0', sizeof(temp));

    printf("\n");

    return 0;
}

int select_daily_checkout_callback(int *socket, int argc, char **argv, char **azColName) {
        char temp[2000];
        printf("\nDB response: %s = %s\n", azColName[0], argv[0] ? argv[0] : "NULL");

        strcpy(temp, "Reservations total: ");
        strncat(temp, argv[0], sizeof(argv[0]));
        strncat(temp, ", Deposits total: ", 20);
        strncat(temp, argv[1], sizeof(argv[1]));
        strncat(temp, "\n", 1);
        send(socket, temp, strlen(temp), 0);
        usleep(1000);
        memset(temp, '\0', sizeof(temp));

    printf("\n");

    return 0;
}


void * handleRequest(thread_struct * str){

char client_message[2000];
char server_message[2000];

 memset(server_message, '\0', sizeof(server_message));
memset(client_message, '\0', sizeof(client_message));

printf("Client connected\n");

pthread_mutex_lock(&lock);

// Receive client's message:
    if (recv(str->client_socket, client_message, sizeof(client_message), 0) < 0){
        printf("Couldn't receive\n");
        return NULL;
    }
    printf("REQUEST from client: %s\n", client_message);

    // Respond to client:
    strcpy(server_message, "SUCCESS");

    if (send(str->client_socket, server_message, strlen(server_message), 0) < 0){

    printf("Can't send\n");
        return;
    }

    if(strcmp(client_message, "GET_TABLES") == 0)
    handleCheckFreeTables(str->client_socket, str->db, str->errMsg);

    else if(strcmp(client_message, "CHECKOUT_DAILY") == 0)
    handleDailyCheckout(str->client_socket, str->db, str->errMsg);

    else if(strcmp(client_message, "CHECKOUT_RES") == 0)
    handleCheckoutClientRequest(str->client_socket, str->db, str->errMsg);

    free(str);
    pthread_mutex_unlock(&lock);
    pthread_exit(NULL);
    return NULL;

}

void handleCheckoutClientRequest(int client_sock, sqlite3 *db, char* err){
char client_message[100];
char server_message[100];
char res_id[10];
char checkout[10];


memset(server_message, '\0', sizeof(server_message));
memset(client_message, '\0', sizeof(client_message));
memset(res_id, '\0', sizeof(res_id));
memset(checkout, '\0', sizeof(checkout));

printf("\nListening for reservation id...");

// Receive client's message:
    if (recv(client_sock, client_message, sizeof(client_message), 0) < 0){
        printf("Couldn't receive\n");
        return -1;
    }
    printf("Retrieved reservation id: %s\n", client_message);
    strcpy(res_id, client_message);
    memset(client_message, '\0', sizeof(res_id));

// Receive client's message:
    if (recv(client_sock, client_message, sizeof(client_message), 0) < 0){
        printf("Couldn't receive\n");
        return -1;
    }
    printf("Retrieve checkout sum: %s", client_message);
    strcpy(checkout, client_message);
    memset(client_message, '\0', sizeof(client_message));

    char sql[100] = {0};

    sprintf(sql, "UPDATE reservation SET total = %s WHERE id = %s", checkout, res_id);

    if(sqlite3_exec(db, sql, 0, 0, &err)!= SQLITE_OK){
            strcpy(server_message, "FAIL");
            if (send(client_sock, server_message, strlen(server_message), 0) < 0){
            printf("Can't send\n");
            return;
        }
        printf("\nDb operation failed!!!: %s", err);
        return;
    }

    strcpy(server_message, "SUCCESS");
    if (send(client_sock, server_message, strlen(server_message), 0) < 0){
    printf("Can't send\n");
        return;
    }
    printf("\nRequest processed successfully");

}

void handleDailyCheckout(int client_socket, sqlite3 *db, char *err_mesg){
char client_message[20];
char server_message[2000];
memset(server_message, '\0', sizeof(server_message));
memset(client_message, '\0', sizeof(client_message));

printf("\nListening for daily chechkout request..");

// Receive client's message:
    if (recv(client_socket, client_message, sizeof(client_message), 0) < 0){
        printf("Couldn't receive\n");
        return;
    }

  printf("\nRequest received. Required date: %s", client_message);
  char sql[100] = {0};

  sprintf(sql, "SELECT SUM(reservation.total), SUM(tables.deposit) FROM reservation INNER JOIN tables ON reservation.table_Id = tables.id WHERE reservation.res_date = \'%s\'", client_message);
  printf("\ndb querry: %s", sql);

  if(sqlite3_exec(db, sql, select_daily_checkout_callback, client_socket, err_mesg)!=SQLITE_OK){
     strcpy(server_message, "FAIL");
            if (send(client_socket, server_message, strlen(server_message), 0) < 0){
            printf("Can't send\n");
            return;
        }
        printf("\nDb operation failed!!!: %s", err_mesg);
        return;
    }

    strcpy(server_message, "SUCCESS");
    if (send(client_socket, server_message, strlen(server_message), 0) < 0){
    printf("Can't send\n");
        return;
    }

  }


void handleCheckFreeTables(int client_socket, sqlite3 *db, char* err_msg){
char client_message[2000];
char server_message[2000];

 memset(server_message, '\0', sizeof(server_message));
memset(client_message, '\0', sizeof(client_message));

    printf("\nClient connected !!!");

    while(1){
    if (recv(client_socket, client_message, sizeof(client_message), 0) < 0){
        printf("Couldn't receive\n");
        return;
    }
        break;
    }

    printf("Requested date from client: %s\n", client_message);

    char sql[120] = "SELECT * FROM tables WHERE id NOT IN (SELECT table_Id from reservation WHERE res_date = \"";
    strncat(sql, client_message, sizeof(client_message));
    strncat(sql, "\")", 5);
    strncat(sql, "\0", 3);

    printf("\n db request: %s", sql);

    int rc = sqlite3_exec(db, sql, select_tables_callback, client_socket, &err_msg);

    // Respond to client:
    strcpy(server_message, "SUCCESS");

    if (send(client_socket, server_message, strlen(server_message), 0) < 0){

    printf("Can't send\n");
        return;
    }

}
