import { Injectable } from '@angular/core';

import { Router } from '@angular/router';
import { Subject } from 'rxjs/Subject';
import { Subscription } from 'rxjs/Subscription';
import 'rxjs/add/operator/filter';
import 'rxjs/add/operator/map';

import { Eventtypes } from '../enum/eventtypes.enum';
import { Logintypes } from '../enum/logintypes.enum';

import {} from './logging/log.service';

interface Message {
  type: Eventtypes;
  payload: any;
}

type MessageCallback = (payload: any) => void;

/**
 *
 * This is a singleton service class. It should only be listed in app.module.ts provider section.  Do not extend from it.
 * It serves a controller for our application and it acts as an event-business which provide publish-subscribe API.
 *
 * Components' constructor should inject this 'ControllerService' class.  Through it, component can publish events and subscribe to events.
 * Thus, inter component communication is made possible. For example:
 *  export class MyComponent implements OnInit, onDestroy {
 *     private subscription: Susbcription;
 *     constructor(private _controllerService : ControllerService  ) {
 *       // subscribe to "Eventtypes.AAA" event
 *        this.subscription= this._controllerService.subscribe( Eventtypes.AAA,
 *                                (payload) => {
 *                                             console.log(payload);
 *                                 });
 *     }
 *     myFunction:(){
 *        let payload = {"username":"Hello"};
 *        this._controllerService.publish(Eventtypes.BBB, payload);  // component's myFunction, it publishs an envent of Eventtypes.BBB type.
 *     }
 *    ngOnDestroy() {
 *        //unsubscribe when component is being destroyed.
 *        this.subscription.unsubscribe();
 *    }
 *   }
 *
 * On the other hand, this 'ControllerService' class also store application-specific data, such as login-user-information, login status, etc.
 * It also functions as a controller.  For example, when login.component receives login-success, login.component just publishs 'LOGIN_SUCCESS' event.
 * As 'ControllerService' subscribes to 'LOGIN_SUCCESS' event, upon receiving 'LOGIN_SUCCESS' message, it will redirect user to other screen.
 *
 * credit to: https://plnkr.co/edit/EjxffJLwTAusMI6BtEUw?p=preview
 */

@Injectable()
export class ControllerService {
  private subscriptions = {};
  private _loginUserInfo: {};

  constructor(
    private _router: Router
  ) {
    this._subscribeToEvents();
  }

  private handler = new Subject<Message>();

  /**
   * publish an event
   */
  publish(type: Eventtypes, payload: any) {
    this.handler.next({ type, payload });
  }

  /**
   * subscribe to an event
   */
  subscribe(type: Eventtypes, callback: MessageCallback): Subscription {
    return this.handler
      .filter(message => message.type === type)
      .map(message => message.payload)
      .subscribe(callback);
  }

    /**
   *  This private function is to subscibe to diffent event.
   */
  private _subscribeToEvents() {
    this.subscriptions[Eventtypes.LOGIN_SUCCESS] = this.subscribe(Eventtypes.LOGIN_SUCCESS, (payload) => {
      this.onLogin(payload);
    });
    this.subscriptions[Eventtypes.NOT_YET_LOGIN] = this.subscribe(Eventtypes.NOT_YET_LOGIN, (payload) => {
      this.onNotYetLogin(payload);
    });
    this.subscriptions[Eventtypes.LOGOUT] = this.subscribe(Eventtypes.LOGOUT, (payload) => {
      this.onLogout(payload);
    });
    this.subscriptions[Eventtypes.SIDENAV_INVOKATION_REQUEST] = this.subscribe(Eventtypes.SIDENAV_INVOKATION_REQUEST, (payload) => {
      this.onSidenavInvokation(payload);
    });
  }

  /**
   * handles case when user login is successful.
   */
  private onLogin(payload: any) {
    this.LoginUserInfo = payload['data'];
    this.directPageAsUserAlreadyLogin();
  }

  /**
   * handles case when user logout event occurs
   */
  private onLogout(payload: any) {


    this._loginUserInfo = null;
    this._router.navigate(['/login']);
  }

   /**
   * handles case when we find user not yet login.
   * For example, before login, user can change browser URL to access page. But we want to block it.
   */
  private onNotYetLogin(payload: any) {
    this._router.navigate(['/login']);
  }

  private onSidenavInvokation(payload) {
    if (payload['id'] === 'workload') {
      this._router.navigate(['/workload']);
    }else {
      this._router.navigate(['/dashboard']);
    }
  }

   /**
   * Getter LoginPatientInfo
   */
  get LoginUserInfo(): {} {
    return this._loginUserInfo;
  }

  /**
   * Setter LoginPatientInfo
   */
  set LoginUserInfo(loginUserInfo: {}) {
    this._loginUserInfo = loginUserInfo;
  }

  public isUserLogin(): boolean {

    // TODO: refine this once user-management REST is available
    return (this.LoginUserInfo && this.LoginUserInfo['username'] !== undefined);
  }

  public directPageAsUserAlreadyLogin() {
   this._router.navigate(['dashboard']);
  }

}
