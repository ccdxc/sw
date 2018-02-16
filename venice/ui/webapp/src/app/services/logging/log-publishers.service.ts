import { Injectable } from '@angular/core';
import { HttpClient, HttpParams , HttpErrorResponse} from '@angular/common/http';
import { Observable } from 'rxjs/Observable';

import { environment } from '../../../environments/environment';
import { LogPublisher, LogConsole, LogLocalStorage, LogWebApi } from './log-publishers';

// ****************************************************
// Logging Publishers Service Class
// ****************************************************
@Injectable()
export class LogPublishersService {
  constructor(private http: HttpClient) {
    // Build publishers arrays
    this.buildPublishers();
  }

  // Public properties
  publishers: LogPublisher[] = [];

  // *************************
  // Public methods
  // *************************
  // Build publishers array
  buildPublishers(): void {
    // Create instance of LogConsole Class
    if (environment.logger_console === true) {
      this.publishers.push(new LogConsole());
    }
    // Create instance of LogLocalStorage Class
    if (environment.logger_localstorage === true) {
      this.publishers.push(new LogLocalStorage());
    }

    // Create instance of LogWebApi Class
    // this.publishers.push(new LogWebApi(this.http));
  }
}
