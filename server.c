// gcc -o server server.c -I /usr/include/postgresql/usr/local/Cellar/postgresql/10.5/include -lpq -std=c99
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <libpq-fe.h>

#define PORT 4444

// Function prototypes:
void getPeopleByCourse(char *token, int *current_socket, PGconn *conn);
void getHabilitiesByCity(char *token, int *current_socket, PGconn *conn);
void getExperiences(char *token, int *current_socket, PGconn *conn);
void setExperience(char *token, int *current_socket, PGconn *conn);

void do_exit(PGconn *conn) {

    PQfinish(conn);
    exit(1);
}

int main(){

	int sockfd, ret;
	struct sockaddr_in serverAddr;

	int newSocket;
	struct sockaddr_in newAddr;

	struct timeval tv1, tv2, elapsed_timeval;

	socklen_t addr_size;

	char buffer[1024];
  char *token;
	pid_t childpid;

  // Database connection
  PGconn *conn = PQconnectdb("user=networks password=networks123 dbname=network_proj_one");

  if (PQstatus(conn) == CONNECTION_BAD) {

      fprintf(stderr, "Connection to database failed: %s\n",
          PQerrorMessage(conn));
      do_exit(conn);
  }

  // Create passive Socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0){
		printf("[-]Error in connection.\n");
		exit(1);
	}
	printf("[+]Server Socket is created.\n");

  // make sure the struct is emty
	memset(&serverAddr, '\0', sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	// serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serverAddr.sin_addr.s_addr = INADDR_ANY;

  // Once you have a socket, you might have to associate that socket with a port on your local machine.
  int optval = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof (optval));

	ret = bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	if(ret < 0){
		printf("[-]Error in binding.\n");
		exit(1);
	}
	printf("[+]Bind to port %d\n", 4444);

	if(listen(sockfd, 2) == 0){
		printf("[+]Listening....\n");
	}else{
		printf("[-]Error in binding.\n");
	}

	while(1){

		newSocket = accept(sockfd, (struct sockaddr*)&newAddr, &addr_size);
		if(newSocket < 0){
			exit(1);
		}

		printf("Connection accepted from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));

		if((childpid = fork()) == 0){
			close(sockfd);
			while(1){
				recv(newSocket, buffer, 1024, 0);
				gettimeofday(&tv1, NULL);

        // Get command and args from buffer:
        printf("Client request: %s\n", buffer);

        /* get the first token */
        token = strtok(buffer, " ");

				if(strcmp(token, ":exit") == 0){
					printf("Disconnected from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
					break;
				}
        else if(strcmp(token, "get-people-by-course") == 0){
					// send(newSocket, token, strlen(token), 0);
          getPeopleByCourse(token, &newSocket, conn);
					bzero(buffer, sizeof(buffer));
				}
        else if(strcmp(token, "get-habilities-by-city") == 0){
          getHabilitiesByCity(token, &newSocket, conn);
					bzero(buffer, sizeof(buffer));
				}
        else if(strcmp(token, "get-experiences") == 0){
          getExperiences(token, &newSocket, conn);
					bzero(buffer, sizeof(buffer));
				}
        else if(strcmp(token, "set-experience") == 0){
          setExperience(token, &newSocket, conn);
          bzero(buffer, sizeof(buffer));
				}
        else if(strcmp(token, ":cu4") == 0){
					printf("Disconnected from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
					break;
				}
        else{
					printf("Client: %s\n", buffer);
					send(newSocket, buffer, strlen(buffer), 0);
					bzero(buffer, sizeof(buffer));
				}
				gettimeofday(&tv2, NULL);
				timersub(&tv1, &tv2, &elapsed_timeval);
				if(elapsed_timeval.tv_sec < 0){
					elapsed_timeval.tv_sec = 0;
				}
				printf("Elapsed time for request/response: %ld:%d (sec:usec)\n", elapsed_timeval.tv_sec, elapsed_timeval.tv_usec);
			}
		}

	}

	close(newSocket);

  PQfinish(conn);

	return 0;
}

void getPeopleByCourse(char *token, int *current_socket, PGconn *conn){

  char *error_answer = "\n----- Error -----\nThe get-people-by-course command requires 1 argument(s):\nget-people-by-course <course-name>\n";
  char *no_rows = "\n----- Query result: People by course -----\n\nNo records found for this course\n";
  int argument_counter = -1;
  int rows = 0;
  char *course;
  char query[2048];
  char response[4096];

  /* walk through other tokens */
  while( token != NULL ) {
    // printf("_%s\n", token);
    argument_counter++;

    // Save the argument
    if(argument_counter == 1){
      course = token;
    }

    token = strtok(NULL, " ");
  }

  // Check number of arguments
  if(argument_counter == 1){

    strcpy(query, "SELECT nome FROM Users where formacao = '");
    strcat(query, course);
    strcat(query, "'");
    PGresult *res = PQexec(conn, query);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {

        printf("No data retrieved\n");
        PQclear(res);
        // Maybe there is no need to close the connection
        // do_exit(conn);
    }
    else{
      rows = PQntuples(res);
      printf("rows: %d\n", rows);

      //Create a header for the response:
      strcpy(response, "\n----- Query result: People by course -----\n\n");

      if(rows > 0){
        for(int i=0; i<rows; i++) {

            printf("%s\n", PQgetvalue(res, i, 0));
            strcat(response, PQgetvalue(res, i, 0));
            strcat(response, "\n");
        }

        // Maybe treat no rows here
        send(*current_socket, &response, strlen(response), 0);
        bzero(response, sizeof(response));
      }
      else{
        send(*current_socket, no_rows, strlen(no_rows), 0);
      }
    }
  }
  else{
    send(*current_socket, error_answer, strlen(error_answer), 0);
  }

  return;
}

// TODO deal with composite city names
void getHabilitiesByCity(char *token, int *current_socket, PGconn *conn){

  char *error_answer = "\n----- Error -----\nThe get-habilities-by-city command requires 1 argument(s):\nget-habilities-by-city <city>\n";
  char *no_rows = "\n----- Query result: Habilities by city -----\n\nNo records found for this city\n";
  int argument_counter = -1;
  int rows = 0;
  char *city;
  char query[2048];
  char response[4096];

  /* walk through other tokens */
  while( token != NULL ) {
    // printf("_%s\n", token);
    argument_counter++;

    // Save the argument
    if(argument_counter == 1){
      city = token;
    }

    token = strtok(NULL, " ");
  }

  // Check number of arguments
  if(argument_counter == 1){

    strcpy(query,
      "SELECT name FROM Users INNER JOIN users_habilities on Users.id = users_habilities.user_id INNER JOIN Habilities on users_habilities.hability_id = Habilities.id where residencia = '"
    );
    strcat(query, city);
    strcat(query, "'");
    PGresult *res = PQexec(conn, query);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {

        printf("No data retrieved\n");
        PQclear(res);
        // Maybe there is no need to close the connection
        // do_exit(conn);
    }
    else{
      rows = PQntuples(res);
      printf("rows: %d\n", rows);

      //Create a header for the response:
      strcpy(response, "\n----- Query result: Habilities by city -----\n\n");

      if(rows > 0){
        for(int i=0; i<rows; i++) {

            printf("%s\n", PQgetvalue(res, i, 0));
            strcat(response, PQgetvalue(res, i, 0));
            strcat(response, "\n");
        }

        // Maybe treat no rows here
        send(*current_socket, &response, strlen(response), 0);
        bzero(response, sizeof(response));
      }
      else{
        send(*current_socket, no_rows, strlen(no_rows), 0);
      }
    }
  }
  else{
    send(*current_socket, error_answer, strlen(error_answer), 0);
  }

  return;
}

void getExperiences(char *token, int *current_socket, PGconn *conn){

  char *error_answer = "\n----- Error -----\nThe get-experiences command requires 1 argument(s):\nget-experiences <email>\n";
  char *no_rows = "\n----- Query result: User's experiences -----\n\nNo records found for this user \n";
  int argument_counter = -1;
  int rows = 0;
  char *email;
  char query[2048];
  char response[4096];

  /* walk through other tokens */
  while( token != NULL ) {
    // printf("_%s\n", token);
    argument_counter++;

    // Save the argument
    if(argument_counter == 1){
      email = token;
    }

    token = strtok(NULL, " ");
  }

  // Check number of arguments
  if(argument_counter == 1){

    strcpy(query,
      "SELECT time, company, name FROM Users inner join Experiences on Users.id = Experiences.user_id where email = '"
    );
    strcat(query, email);
    strcat(query, "'");
    PGresult *res = PQexec(conn, query);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {

        printf("No data retrieved\n");
        PQclear(res);
        // Maybe there is no need to close the connection
        // do_exit(conn);
    }
    else{
      rows = PQntuples(res);
      printf("rows: %d\n", rows);

      //Create a header for the response:
      strcpy(response, "\n----- Query result: User's experiences -----\n\n");

      if(rows > 0){
        for(int i=0; i<rows; i++) {

            printf("%s\n", PQgetvalue(res, i, 0));
            strcat(response, "Trabalhou ");
            strcat(response, PQgetvalue(res, i, 0));
            strcat(response, " ano(s) na empresa ");
            strcat(response, PQgetvalue(res, i, 1));
            strcat(response, " como ");
            strcat(response, PQgetvalue(res, i, 2));
            strcat(response, "\n");
        }

        // Maybe treat no rows here
        send(*current_socket, &response, strlen(response), 0);
        bzero(response, sizeof(response));
      }
      else{
        send(*current_socket, no_rows, strlen(no_rows), 0);
      }
    }
  }
  else{
    send(*current_socket, error_answer, strlen(error_answer), 0);
  }

  return;
}

void setExperience(char *token, int *current_socket, PGconn *conn){

  char *error_answer = "\n----- Error -----\nThe set-experience command requires 4 argument(s):\nset-experiences <user-id> <job-title> <time> <company>\n";
  char *no_rows = "\n----- Query result: Set user experience -----\n\nNo records found for this user \n";
  int argument_counter = -1;
  int rows = 0;
  char *user_id;
  char *job_title;
  char *time;
  char *company;
  char query[2048];
  char response[4096];

  /* walk through other tokens */
  while( token != NULL ) {
    // printf("_%s\n", token);
    argument_counter++;

    // Save the arguments
    if(argument_counter == 1){
      user_id = token;
    }
    if(argument_counter == 2){
      job_title = token;
    }
    if(argument_counter == 3){
      time = token;
    }
    if(argument_counter == 4){
      company = token;
    }

    token = strtok(NULL, " ");
  }

  // Check number of arguments
  if(argument_counter == 4){

    strcpy(query,
      "INSERT INTO Experiences (user_id, name, time, company) VALUES("
    );
    strcat(query, user_id);
    strcat(query, ",'");
    strcat(query, job_title);
    strcat(query, "',");
    strcat(query, time);
    strcat(query, ",'");
    strcat(query, company);
    strcat(query, "')");
    printf("Query: %s\n", query);
    PGresult *res = PQexec(conn, query);

    if (PQresultStatus(res) != PGRES_COMMAND_OK){
      printf("\n----- Error: Could not insert Experience for user -----\n");
      PQclear(res);
    }
    else{

      strcpy(response, "\n----- Success! Record Inserted -----\n\n");
      // Maybe treat no rows here
      send(*current_socket, &response, strlen(response), 0);
      bzero(response, sizeof(response));
    }
  }
  else{
    send(*current_socket, error_answer, strlen(error_answer), 0);
  }

  return;
}
