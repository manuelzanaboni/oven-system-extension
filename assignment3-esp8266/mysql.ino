// MySQL libraries
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>

// MySQL server cfg
char mysql_user[] = MYSQL_USER;       // MySQL user login username
char mysql_password[] = MYSQL_PASS;   // MySQL user login password
IPAddress server_addr(MYSQL_IP);      // IP of the MySQL *server* here
MySQL_Connection conn((Client *)&client);

char query[128];
char INSERT_EVENT[] = "INSERT INTO `mzanaboni`.`events` (`who`, `type`) VALUES ('%s', '%s')";

int write_event_to_db(char who[], char type[]) {
  int error;
  if (conn.connect(server_addr, 3306, mysql_user, mysql_password)) {
    Serial.println(F("MySQL connection established."));

    MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);

    sprintf(query, INSERT_EVENT, who, type);
    Serial.println(query);
    // execute the query
    cur_mem->execute(query);
    delete cur_mem;
    error = 1;
    Serial.println(F("Data recorded on MySQL"));

    conn.close();
  } else {
    Serial.println(F("MySQL connection failed."));
    error = -1;
  }

  return error;
}
