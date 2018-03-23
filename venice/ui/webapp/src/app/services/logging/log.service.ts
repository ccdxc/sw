import { Injectable } from '@angular/core';

import { LogPublishersService } from './log-publishers.service';
import { LogPublisher } from './log-publishers';
import { environment } from '../../../environments/environment';

// ****************************************************
// Log Level Enumeration
// ****************************************************
export enum LogLevel {
  All = 0,
  Debug = 1,
  Info = 2,
  Warn = 3,
  Error = 4,
  Fatal = 5,
  Off = 6
}

// ****************************************************
// Log Entry Class
// ****************************************************
export class LogEntry {
  // Public Properties
  entryDate: Date = new Date();
  message = '';
  level: LogLevel = (environment.log_level !== undefined) ? environment.log_level : LogLevel.Debug;
  extraInfo: any[] = [];
  logWithDate = true;
  caller = '';

  // **************
  // Public Methods
  // **************
  buildLogToString(): string {
    let value = '';

    if (this.logWithDate) {
      const today = new Date();
      value = today.toISOString() + ' - ';
    }
    value += 'DebugType: ' + LogLevel[this.level];

    value += ' - Caller: ' + this.caller;

    value += ' - Message: ' + this.message;
    if (this.extraInfo.length) {
      value += ' - Extra Info: '
        + this.formatParams(this.extraInfo);
    }
    return value;
  }

  buildLogString(): string {
     if (environment.log_format === 'JSON') {
       return this.buildLogToJSON();
     }
     return this.buildLogToString();
  }

  buildLogToJSON(): string {
    let value = '';
    const outObj = {};
    const today = new Date();
    outObj['caller'] = this.caller;
    outObj['mesage'] =  this.message;
    if (this.extraInfo.length) {
      value += this.formatParams(this.extraInfo);
    }
    outObj['extrainfo'] = value;
    outObj['time'] = today.toISOString() ;
    outObj['debuglevel'] = LogLevel[this.level];
    return JSON.stringify(outObj, null, 1);
  }

  // ***************
  // Private Methods
  // ***************
  private formatParams(params: any[]): string {
    let ret: string = params.join(',');

    // Is there at least one object in the array?
    if (params.some(p => typeof p === 'object')) {
      ret = '';
      // Build comma-delimited string
      for (const item of params) {
        let stringifyOutput = '';
        try {
          stringifyOutput = JSON.stringify(item) ;
        } catch (err) {
          stringifyOutput = item.toString();
        }

        ret += stringifyOutput + ',';
      }
    }

    return ret;
  }



}

// ****************************************************
// Log Service Class
// ****************************************************
@Injectable()
export class LogService {
  constructor(private publishersService: LogPublishersService) {
    // Set publishers
    this.publishers = this.publishersService.publishers;
  }

  // Public Properties
  publishers: LogPublisher[];
  level: LogLevel = LogLevel.All;
  logWithDate = true;

  // *************************
  // Public methods
  // *************************
  debug(msg: string, caller?: string, ...optionalParams: any[]) {
    this.writeToLog(msg, LogLevel.Debug, caller, optionalParams);
  }

  info(msg: string, caller?: string, ...optionalParams: any[]) {
    this.writeToLog(msg, LogLevel.Info, caller,  optionalParams);
  }

  warn(msg: string, caller?: string, ...optionalParams: any[]) {
    this.writeToLog(msg, LogLevel.Warn, caller,  optionalParams);
  }

  error(msg: string, caller?: string, ...optionalParams: any[]) {
    this.writeToLog(msg, LogLevel.Error, caller,  optionalParams);
  }

  fatal(msg: string, caller?: string, ...optionalParams: any[]) {
    this.writeToLog(msg, LogLevel.Fatal,  caller, optionalParams);
  }

  log(msg: string, caller?: string, ...optionalParams: any[]) {
    this.writeToLog(msg, LogLevel.All,  caller, optionalParams);
  }

  clear(): void {
    for (const logger of this.publishers) {
      logger.clear()
        .subscribe(response => console.log(response));
    }
  }

  // *************************
  // Private methods
  // *************************
  private shouldLog(level: LogLevel): boolean {
    let ret = false;


    if ((level >= this.level &&
      level !== LogLevel.Off) ||
      this.level === LogLevel.All) {
      ret = true;
    }

    return ret;
  }

  private writeToLog(msg: string, level: LogLevel,  caller: string , params: any[]) {
    if (this.shouldLog(level)) {
      // Declare variables
      const entry: LogEntry = new LogEntry();

      // Build Log Entry
      entry.message = msg;
      entry.level = level;
      entry.extraInfo = params;
      entry.logWithDate = this.logWithDate;
      if (caller) {
        entry.caller = caller;
      }

      for (const logger of this.publishers) {
        logger.log(entry)
          .subscribe(response => {

          });
      }
    }
  }
}
