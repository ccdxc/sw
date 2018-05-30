import { Component, OnInit } from '@angular/core';
import { Utility } from './common/Utility';
import { environment } from '../environments/environment';
import { LogService } from './services/logging/log.service';

@Component({
  selector: 'app-common',
  template: '<div></div>'
})

/**
 * This is the root Component of application.
 *
 * Put shared data in this class.
 */
export class CommonComponent implements OnInit {
  protected subscriptions = {};
  protected successMessage = '';
  protected errorMessage = '';
  protected autosaveMessage = { 'status': true, 'message': '' };
  public isRESTServerReady = false;

  protected logger: LogService;

  constructor() {
    this.isRESTServerReady = environment.isRESTAPIReady;
    // Below line is needed
    this.logger = Utility.getInstance().getLogService();
  }

  ngOnInit() { }

  /**
   * Get the component class-name
   */
  getClassName(): string {
    return '';

  }

  // *************************
  // Public methods
  // *************************
  debug(msg: string, ...optionalParams: any[]) {
    if (!this.logger) {
      console.error('common.component.ts logger is null');
      return;
    }
    const caller = this.getClassName();
    this.logger.debug(msg, caller, optionalParams);
  }

  info(msg: string, ...optionalParams: any[]) {
    if (!this.logger) {
      console.error('common.component.ts logger is null');
      return;
    }
    const caller = this.getClassName();
    this.logger.info(msg, caller, optionalParams);
  }

  warn(msg: string, ...optionalParams: any[]) {
    if (!this.logger) {
      console.error('common.component.ts logger is null');
      return;
    }
    const caller = this.getClassName();
    this.logger.warn(msg, caller, optionalParams);
  }

  error(msg: string, ...optionalParams: any[]) {
    if (!this.logger) {
      console.error('common.component.ts logger is null');
      return;
    }
    const caller = this.getClassName();
    this.logger.error(msg, caller, optionalParams);
  }

  fatal(msg: string, ...optionalParams: any[]) {
    if (!this.logger) {
      console.error('common.component.ts logger is null');
      return;
    }
    const caller = this.getClassName();
    this.logger.fatal(msg, caller, optionalParams);
  }

  log(msg: string, ...optionalParams: any[]) {
    if (!this.logger) {
      console.error('common.component.ts logger is null');
      return;
    }
    const caller = this.getClassName();
    this.logger.log(msg, caller, optionalParams);
  }

  clear(): void {
    if (!this.logger) {
      console.error('common.component.ts logger is null');
      return;
    }
    this.logger.clear();
  }

  protected unsubscribeAll() {
    Object.keys(this.subscriptions).forEach((item) => {
      if (this.subscriptions[item]) {
        this.subscriptions[item].unsubscribe();
      }
    });
  }
}
