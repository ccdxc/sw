import { Injectable } from '@angular/core';
import { ReplaySubject } from 'rxjs';
import { ControllerService } from '@app/services/controller.service';
import { Eventtypes } from '@app/enum/eventtypes.enum';

const APP_PREFIX = 'app-';

export enum LocalStorageEvents {
  SENDING_LOGIN_DATA = 'sendingLoginData',
  GET_LOGIN_DATA = 'getLoginData',
  NEW_LOGIN_DATA = 'newLogin',
  LOGOUT_REQUEST = 'logoutRequest'
}

/**
 * We keep the user's authentication in each tabs sessionStorage.
 * In order to prevent the user from logging in again on
 * opening a second tab, on a new tab we copy a previous tabs
 * auth token into local storage, then into the new tab, and then
 * erase it from local storage.
 *
 * Adapted from
 * https://stackoverflow.com/questions/20325763/browser-sessionstorage-share-between-tabs?answertab=active#tab-top
 *
 * When a new tab is created it fires a new tab event.
 * Other tabs that exist are listening for this event, and
 * will send over there session variables. The new tab will
 * copy over the session variable. If it is on the login page,
 * there is a subscription to an observable in this class so that
 * the login component can know when to recheck the session variables
 * for credentials.
 *
 * If a user ever for some reason gets a new token, it will
 * emit its token to all the other tabs.
 *
 * If a user logs out of one tab, all the other tabs will be notified
 * and will also log out.
 * 
 * The tokens are always kept in session storage, and only briefly
 * put into local storage when sending to another tab.
 *
 */
@Injectable()
export class LocalStorageService {
  isStorageAvailable: boolean = true;

  // When a signed in tab is closed, if a user
  // uses a browser's "reopen close tab" feature
  // some browsers also restore the session variables
  // of the tab instead of having a blank slate, which you would
  // have if you had manually opened another tab.
  // This is slightly less secure (user closes tab, and another user
  // comes and reopens it to get access without having to login), but
  // most products (gmail, facebook...) allow this behavior. This can be
  // toggled with the following variable.
  preventSessionRestore: boolean = false;

  // Used to notify any listeners that we have new
  // login data
  observable = new ReplaySubject();

  static loadInitialState() {
    return Object.keys(localStorage).reduce((state: any, storageKey) => {
      if (storageKey.includes(APP_PREFIX)) {
        state = state || {};
        const stateKey = storageKey
          .replace(APP_PREFIX, '')
          .toLowerCase()
          .split('.');
        let currentStateRef = state;
        stateKey.forEach((key, index) => {
          if (index === stateKey.length - 1) {
            currentStateRef[key] = JSON.parse(localStorage.getItem(storageKey));
            return;
          }
          currentStateRef[key] = currentStateRef[key] || {};
          currentStateRef = currentStateRef[key];
        });
      }
      return state;
    }, undefined);
  }

  constructor(protected controllerService: ControllerService) {
    if (!this.storageAvailable()) {
      this.isStorageAvailable = false;
      return;
    }
    // Ask other tabs for session storage (this is ONLY to trigger event)
    if (!sessionStorage.length) {
      this.setItem(LocalStorageEvents.GET_LOGIN_DATA, 'newtab');
      this.removeItem(LocalStorageEvents.GET_LOGIN_DATA);
    }
    // listen for changes to localStorage
    window.addEventListener('storage', (event) => {
      this.sessionStorageTransfer(event);
    }, false);
    window.addEventListener('beforeunload', () => {
      this.tabRemoval();
    }, false);
  }

  sessionStorageTransfer(event) {
    if (!event) { event = window.event; } // for ie
    if (event.key == null || !event.key.startsWith(APP_PREFIX)) { return; } // Event isn't related to our app.
    const eventKey = event.key.slice(APP_PREFIX.length);
    if (eventKey === LocalStorageEvents.GET_LOGIN_DATA) {
      // another tab asked for the sessionStorage -> send it
      this.setItem(LocalStorageEvents.SENDING_LOGIN_DATA, sessionStorage);
      // Session storage calls are synchronous so the
      // other tab should now have it, so we're done with it.
      this.removeItem(LocalStorageEvents.SENDING_LOGIN_DATA);
    } else if (
      (eventKey === LocalStorageEvents.SENDING_LOGIN_DATA && !sessionStorage.length) ||
      (eventKey === LocalStorageEvents.NEW_LOGIN_DATA)) {
      // This tab doesn't have any data and another tab has sent us data
      // OR
      // There was a new login, and we need to take update our
      // tokens
      if (!event.newValue) { return; }          // do nothing if no value to work with
      const data = JSON.parse(event.newValue);
      for (const key of Object.keys(data)) {
        sessionStorage.setItem(key, data[key]);
      }
      // In we are on the login page, we should be
      // subscribed to this observable, and this will
      // let us know to check our login status again.
      this.observable.next(true);
    } else if (eventKey === LocalStorageEvents.LOGOUT_REQUEST && sessionStorage.length) {
      // User logged out on another tab, we logout on this one.
      // To prevent an infinite loop, we check if sessionStore is populated
      // sessionStore shoudn't be populated if we are the login screen.
      this.controllerService.publish(Eventtypes.LOGOUT, { 'reason': 'User logged out of another tab.' });
    }
  }

  /**
   * Returns whether local storage is available
   * Taken from
   * https://developer.mozilla.org/en-US/docs/Web/API/Web_Storage_API/Using_the_Web_Storage_API
   */
  private storageAvailable() {
    const type = 'localStorage';
    const storage = window[type];
    try {
      const x = '__storage_test__';
      storage.setItem(x, x);
      storage.removeItem(x);
      return true;
    } catch (e) {
      return e instanceof DOMException && (
        // everything except Firefox
        e.code === 22 ||
        // Firefox
        e.code === 1014 ||
        // test name field too, because code might not be present
        // everything except Firefox
        e.name === 'QuotaExceededError' ||
        // Firefox
        e.name === 'NS_ERROR_DOM_QUOTA_REACHED') &&
        // acknowledge QuotaExceededError only if there's something already stored
        storage.length !== 0;
    }
  }

  tabRemoval() {
    if (this.preventSessionRestore) {
      // Remove session storage
      sessionStorage.clear();
    }
  }


  /**
   * Emits when we have login data in our session storage
   */
  getUserdataObservable() {
    return this.observable;
  }

  setItem(key: string, value: any) {
    if (this.isStorageAvailable) {
      localStorage.setItem(`${APP_PREFIX}${key}`, JSON.stringify(value));
    }
  }

  removeItem(key: string) {
    if (this.isStorageAvailable) {
      localStorage.removeItem(`${APP_PREFIX}${key}`);
    }
  }

  getItem(key: string) {
    if (this.isStorageAvailable) {
      return JSON.parse(localStorage.getItem(`${APP_PREFIX}${key}`));
    }
    return null;
  }
}
