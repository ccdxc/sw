import {
  ApplicationRef,
  ComponentFactoryResolver,
  EmbeddedViewRef,
  Injectable,
  Injector,
  NgModuleFactoryLoader,
} from '@angular/core';
import { MatIconRegistry } from '@angular/material';
import { DomSanitizer } from '@angular/platform-browser';
import { Router } from '@angular/router';
import { Eventtypes } from '@app/enum/eventtypes.enum';
import { PinPayload } from '@app/models/frontend/shared/pinpayload.interface.ts';
import { ToolbarData, ToolbarButton } from '@app/models/frontend/shared/toolbar.interface.ts';
import * as _ from 'lodash';
import { Promise } from 'q';
import { Subject, Subscription } from 'rxjs';
import { filter, map } from 'rxjs/operators';
import { LogService } from '@app/services/logging/log.service';
import { AUTH_KEY, AUTH_BODY } from '@app/core/auth/auth.reducer';
import { ConfirmationService, Confirmation } from 'primeng/primeng';
import { MessageService } from '@app/services/message.service';
import { Utility } from '@app/common/Utility';
import { HelpData } from '@app/models/frontend/shared/helpdata.interface';

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
  private toolbarData: ToolbarData = {};
  // Keeping help data format as any until we have
  // a better idea of what this looks like.
  private helpData: HelpData = {};
  private userSettings: any = {
    dashboardPinned: []
  };
  public useRealData = false;
  private _enableIdle = true;
  // time to idle before warning user of logout (in seconds)
  private _idleTime = 60 * 60 * 1;

  constructor(
    private _router: Router,
    private _injector: Injector,
    private _resolver: ComponentFactoryResolver,
    private _appRef: ApplicationRef,
    private loader: NgModuleFactoryLoader,
    protected domSanitizer: DomSanitizer,
    protected matIconRegistry: MatIconRegistry,
    protected logger: LogService,
    protected messageService: MessageService,
    protected confirmService: ConfirmationService,
  ) {
    this._subscribeToEvents();
    this._registerSVGIcons();
  }

  private handler = new Subject<Message>();

  _registerSVGIcons() {
    // add custom material icons
    this.matIconRegistry.addSvgIcon('menu-pin', this.domSanitizer.bypassSecurityTrustResourceUrl('/assets/images/icons/icon-pin-menu.svg'));
    this.matIconRegistry.addSvgIcon('menu-expand', this.domSanitizer.bypassSecurityTrustResourceUrl('/assets/images/icons/icon-expand-menu.svg'));
    this.matIconRegistry.addSvgIcon('menu-collapse', this.domSanitizer.bypassSecurityTrustResourceUrl('/assets/images/icons/icon-collapse-menu.svg'));
    this.matIconRegistry.addSvgIcon('venice', this.domSanitizer.bypassSecurityTrustResourceUrl('/assets/images/icons/icon-venice.svg'));
    this.matIconRegistry.addSvgIcon('policy-health', this.domSanitizer.bypassSecurityTrustResourceUrl('/assets/images/icons/icon-policyhealth.svg'));
    this.matIconRegistry.addSvgIcon('policy', this.domSanitizer.bypassSecurityTrustResourceUrl('/assets/images/icons/ico-policy.svg'));
    this.matIconRegistry.addSvgIcon('security-policy', this.domSanitizer.bypassSecurityTrustResourceUrl('/assets/images/icons/icon-security-policy.svg'));
    this.matIconRegistry.addSvgIcon('security', this.domSanitizer.bypassSecurityTrustResourceUrl('/assets/images/icons/security/ico-security.svg'));

    // network icons
    this.matIconRegistry.addSvgIcon('network-network', this.domSanitizer.bypassSecurityTrustResourceUrl('/assets/images/icons/network/ico-network-network.svg'));
    this.matIconRegistry.addSvgIcon('network-lbpolicy', this.domSanitizer.bypassSecurityTrustResourceUrl('/assets/images/icons/network/ico-network-loadbalance_policy.svg'));

    // security icons
    this.matIconRegistry.addSvgIcon('security-group', this.domSanitizer.bypassSecurityTrustResourceUrl('/assets/images/icons/security/ico-security-group.svg'));
    this.matIconRegistry.addSvgIcon('security-policy', this.domSanitizer.bypassSecurityTrustResourceUrl('/assets/images/icons/security/icon-security-policy-black.svg'));

    // Workload icons
    this.matIconRegistry.addSvgIcon('workloads-alert', this.domSanitizer.bypassSecurityTrustResourceUrl('/assets/images/icons/workload/icon-workloads-alert.svg'));
    this.matIconRegistry.addSvgIcon('workloads-new', this.domSanitizer.bypassSecurityTrustResourceUrl('/assets/images/icons/workload/icon-workloads-new.svg'));
    this.matIconRegistry.addSvgIcon('workloads-total', this.domSanitizer.bypassSecurityTrustResourceUrl('/assets/images/icons/workload/icon-workloads-total.svg'));
    this.matIconRegistry.addSvgIcon('workloads-unprotected', this.domSanitizer.bypassSecurityTrustResourceUrl('/assets/images/icons/workload/icon-workloads-unprotected.svg'));
    this.matIconRegistry.addSvgIcon('workload', this.domSanitizer.bypassSecurityTrustResourceUrl('/assets/images/icons/workload/icon-workload.svg'));
    this.matIconRegistry.addSvgIcon('workloads', this.domSanitizer.bypassSecurityTrustResourceUrl('/assets/images/icons/workload/icon-workloads.svg'));

    // Cluster
    this.matIconRegistry.addSvgIcon('cluster', this.domSanitizer.bypassSecurityTrustResourceUrl('/assets/images/icons/cluster/ico-cluster.svg'));
    this.matIconRegistry.addSvgIcon('node', this.domSanitizer.bypassSecurityTrustResourceUrl('/assets/images/icons/cluster/nodes/ico-node.svg'));
    this.matIconRegistry.addSvgIcon('memory', this.domSanitizer.bypassSecurityTrustResourceUrl('/assets/images/icons/cluster/icon-memory.svg'));
    this.matIconRegistry.addSvgIcon('cpu', this.domSanitizer.bypassSecurityTrustResourceUrl('/assets/images/icons/cluster/icon-cpu.svg'));
    this.matIconRegistry.addSvgIcon('storage', this.domSanitizer.bypassSecurityTrustResourceUrl('/assets/images/icons/cluster/icon-storage.svg'));
    this.matIconRegistry.addSvgIcon('host', this.domSanitizer.bypassSecurityTrustResourceUrl('/assets/images/icons/cluster/ico-host.svg'));

    // Naple/DSC icons
    this.matIconRegistry.addSvgIcon('naples', this.domSanitizer.bypassSecurityTrustResourceUrl('/assets/images/icons/cluster/naples/ico-dsc-white.svg'));


    this.matIconRegistry.addSvgIcon('un-pin', this.domSanitizer.bypassSecurityTrustResourceUrl('/assets/images/dashboard/ico-unpinned.svg'));
    this.matIconRegistry.addSvgIcon('auth-pol', this.domSanitizer.bypassSecurityTrustResourceUrl('/assets/images/icons/admin/ico-auth-pol.svg'));

    this.matIconRegistry.addSvgIcon('drag_indicator', this.domSanitizer.bypassSecurityTrustResourceUrl('/assets/images/icons/baseline-drag_indicator-24px.svg'));

    this.matIconRegistry.addSvgIcon('card_admit', this.domSanitizer.bypassSecurityTrustResourceUrl('/assets/images/icons/networkgraph/ico-admission.svg'));
    this.matIconRegistry.addSvgIcon('card_decommision', this.domSanitizer.bypassSecurityTrustResourceUrl('/assets/images/icons/networkgraph/ico-deactivate.svg'));
  }

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
    return this.handler.pipe(
      filter(message => message.type === type),
      map(message => message.payload)
    ).subscribe(callback);
  }

  /**
   *  This private function is to subscibe to diffent event.
   */
  private _subscribeToEvents() {
    this.subscriptions[Eventtypes.PIN_REQUEST] = this.subscribe(Eventtypes.PIN_REQUEST, (payload) => {
      this.pinToDashboard(payload);
    });
    this.subscriptions[Eventtypes.UNPIN_REQUEST] = this.subscribe(Eventtypes.UNPIN_REQUEST, (payload) => {
      this.unpinFromDashboard(payload);
    });
  }

  /**
  * Getter LoginUser object
  */
  get LoginUserInfo(): {} {
    if (!this._loginUserInfo) {
      this._loginUserInfo = JSON.parse(sessionStorage.getItem(AUTH_BODY));
    }
    return this._loginUserInfo;
  }

  /**
   * Setter LoginUser
   */
  set LoginUserInfo(loginUserInfo: {}) {
    this._loginUserInfo = loginUserInfo;
  }

  public isUserLogin(): boolean {
    return sessionStorage.getItem(AUTH_KEY) != null;
  }


  public directPageAsUserAlreadyLogin() {
    this._router.navigate(['dashboard']);
  }

  /***** Getting and setting toolbar data *****/
  public getToolbarData(): any {
    return _.cloneDeep(this.toolbarData);
  }

  public setToolbarData(data: ToolbarData) {
    if (!_.isEqual(this.toolbarData, data)) {
      this.toolbarData = _.cloneDeep(data);
      this.publish(Eventtypes.TOOLBAR_DATA_CHANGE, data);
    }
  }

  public getHelpOverlayData(): HelpData {
    return this.helpData;
  }

  public setHelpOverlayData(data: HelpData) {
    if (!_.isEqual(this.helpData, data)) {
      this.helpData = data;
      this.publish(Eventtypes.HELP_CONTENT_CHANGE, data);
    }
  }

  public getCurrentRoute() {
    return this._router.url;
  }

  public navigate(paths: string[]) {
    this._router.navigate(paths);
  }

  /***** Building component from another module *****/

  /**
   * This method creates component by component's name
   * e.g
   * let compoent = this._controllerService.createComponent('BaseComponent');
   *
   * @param comp
   */
  instantiateComponent(comp: string): any {
    const factories = Array.from((<any>this._resolver)['_factories'].keys());
    const factoryClass = factories.find((x: any) => x.name === comp);
    const component = <any>factoryClass;
    this._resolver.resolveComponentFactory(component);

    const componentRef = this._resolver.resolveComponentFactory(component).create(this._injector);

    return componentRef;

  }

  /**
   * Append a component to HTML DOM Element
   * @param componentRef
   * @param htmlElemennt
   */
  appendComponentToDOMElement(componentRef: any, htmlElemennt: HTMLElement) {

    // Attach component to the appRef so that it's inside the ng component tree
    this._appRef.attachView(componentRef.hostView);

    // Get DOM element from component
    const domElem = (componentRef.hostView as EmbeddedViewRef<any>).rootNodes[0] as HTMLElement;

    // Append DOM element to the HTML DOM Element
    htmlElemennt.appendChild(domElem);
  }

  /**
   *
   * @param moduleName
   * @param componentName
   */
  _buildComponentFromModuleHelper(moduleName: string, componentName: string) {
    return Promise((resolve) => {
      this.loader.load(moduleName).then((factory) => {
        const module = factory.create(this._injector);
        const r = module.componentFactoryResolver;
        const factories = Array.from((<any>r)['_factories'].keys());
        const factoryClass = factories.find((x: any) => x.name === componentName);
        const component = <any>factoryClass;
        const cmpFactory = r.resolveComponentFactory(component);

        // create a component and attach it to the view
        const componentRef = cmpFactory.create(this._injector);
        resolve(componentRef);

      });
    });
  }

  /**
   * Instantiate a component defined in a module. The module is a lazy-loading module and it may not be loaded in application yet.
   * For example:
   *
   * this._controllerService.buildComponentFromModule('@components/workload/workload.module#WorkloadModule', 'WorkloadwidgetComponent')
   *   .then((component) => {
   *    // change component properties and append component to UI view
   *     this._componentSetup(component);
   *     this._controllerService.appendComponentToDOMElement(component, document.body);
   *   });
   * @param moduleName
   * @param componentName
   */
  buildComponentFromModule(moduleName: string, componentName: string) {
    return this._buildComponentFromModuleHelper(moduleName, componentName).then((componentRef) => {
      return componentRef;
    });
  }

  /***** pinning ******/

  getPinnedDashboardItems() {
    return this.userSettings.dashboardPinned;
  }

  pinToDashboard(payload: PinPayload) {
    // Check if item already exists
    if (this.alreadyPinned(payload)) {
      return false;
    }
    this.userSettings.dashboardPinned.push(payload);
    return true;
  }

  unpinFromDashboard(payload: PinPayload) {
    // Check if item is pinned
    if (!this.alreadyPinned(payload)) {
      return false;
    }
    _.remove(this.userSettings.dashboardPinned, (i) => _.isEqual(i, payload));
    return true;
  }

  alreadyPinned(payload: PinPayload) {
    for (const item of this.userSettings.dashboardPinned) {
      if (_.isEqual(item, payload)) {
        return true;
      }
    }
    return false;
  }

  /***** idle feature *****/

  /**
   * Getter LoginPatientInfo
   */
  get enableIdle(): boolean {
    return this._enableIdle;
  }

  /**
   * Setter LoginPatientInfo
   */
  set enableIdle(enableIdle: boolean) {
    if (enableIdle !== this._enableIdle) {
      this._enableIdle = enableIdle;
      this.publish(Eventtypes.IDLE_CHANGE, { active: enableIdle });
    }
  }

  /**
   * Getter LoginPatientInfo
   */
  get idleTime(): number {
    return this._idleTime;
  }

  /**
   * Setter LoginPatientInfo
   */
  set idleTime(seconds: number) {
    if (seconds !== this._idleTime) {
      this._idleTime = seconds;
      this.publish(Eventtypes.IDLE_CHANGE, { time: seconds });
    }
  }

  /**
   * Logging methods
   */
  debug(msg: string, caller: string, ...optionalParams: any[]) {
    this.logger.debug(msg, caller, optionalParams);
  }

  info(msg: string, caller: string, ...optionalParams: any[]) {
    this.logger.info(msg, caller, optionalParams);
  }

  warn(msg: string, caller: string, ...optionalParams: any[]) {
    this.logger.warn(msg, caller, optionalParams);
  }

  error(msg: string, caller: string, ...optionalParams: any[]) {
    this.logger.error(msg, caller, optionalParams);
  }

  fatal(msg: string, caller: string, ...optionalParams: any[]) {
    this.logger.fatal(msg, caller, optionalParams);
  }

  log(msg: string, caller: string, ...optionalParams: any[]) {
    this.logger.log(msg, caller, optionalParams);
  }

  clear(): void {
    this.logger.clear();
  }

  // Toaster methods
  invokeSuccessToaster(summary: string, detail: string, clearErrorEditToasters: boolean = true, buttons: ToolbarButton[] = []) {
    if (clearErrorEditToasters) {
      this.messageService.clear();
    }
    this.messageService.add({
      severity: 'success',
      summary: summary,
      detail: detail,
      buttons: buttons,
    });
  }

  invokeInfoToaster(summary: string, detail: string, buttons: ToolbarButton[] = []) {
    this.messageService.add({
      severity: 'info',
      summary: summary,
      detail: detail,
      buttons: buttons
    });
  }

  // Removes any toaster that has the same summary and detail
  // If detail is blank, will remove any toaster with the same summary
  removeToaster(summary: string, detail: string = null) {
    this.messageService.remove({
      summary: summary,
      detail: detail
    });
  }

  createSignOutButton() {
    return {
      text: 'Login',
      callback: () => {
        this.publish(Eventtypes.LOGOUT, { 'reason': 'User logged out' });
      },
      cssClass: 'global-button-primary'
    };
  }

  invokeRESTErrorToaster(summary: string, error: any, removeSameSummary: boolean = true) {

    if (Utility.getInstance().getMaintenanceMode()) {
      return;
    }

    if (error == null) {
      return;
    }
    if (error.statusCode == null) {
      error.statusCode = 0;
    }
    if (removeSameSummary) {
      // Remove any toasters that already have this summary
      this.removeToaster(summary);
    }
    const buttons: ToolbarButton[] = [
      this.createSignOutButton()
    ];
    if (error.statusCode === 401) {
      this.invokeErrorToaster(Utility.VENICE_CONNECT_FAILURE_SUMMARY, 'Your credentials have expired. Please sign in again.', buttons);
      return;
    } else if (error.statusCode === 403) {
      this.invokeErrorToaster(Utility.VENICE_CONNECT_FAILURE_SUMMARY, 'Your authorization is insufficient. Please check with your system administrator.');
      return;
    } else if (error.statusCode >= 500) {
      this.invokeErrorToaster(Utility.VENICE_CONNECT_FAILURE_SUMMARY, 'Venice encountered an internal error. Some services may be temporarily unavailable.');
      return;
    }

    if (error.statusCode !== 0) {
      // If status code is 400, we should almost always have a body with error message
      let errorMsg = error.body != null ? error.body.message : 'Bad request';
      if (errorMsg instanceof Array) {
        errorMsg = errorMsg.join('\n');
      }
      this.invokeErrorToaster(summary, errorMsg);
      return;
    } else if (error && error.constructor && error.constructor.name === 'CloseEvent') {
      // VS-478. Run into web-socket issue. See https://stackoverflow.com/questions/19304157/getting-the-reason-why-websockets-closed-with-close-code-1006 (Chrome)
      const errorCode  = (error.code) ? error.code : '';
      const errorMsg = 'Connection close - ' + errorCode;  // error.code is likely 1006, 1011
      this.invokeErrorToaster( 'Web socket', summary + '\n' + errorMsg);
      return;
    }
    // Don't know what the error is, websockets can come to here.
    console.error('controller.service.invokeRESTErrorToaster() \n' + JSON.stringify(error)); // VS-478 display error.
    let msg = 'Your credentials are expired/insufficient or Venice is temporarily unavailable. Please reload browser and sign in again or contact system administrator.';
    if (error.statusCode === 0) {
      // VS-872. After browser idle for some time. It runs into refused to execute inline script because it violates the following content security policy directive...
      // This is due to cert and content-security-policy issue. We advice user to clear broswer cache. (It is conner case and is likely to occur in sys-test.)
       msg = 'Please clear your browser cache and sign in again.';
    }
    this.invokeErrorToaster(Utility.VENICE_CONNECT_FAILURE_SUMMARY, msg, buttons);
  }

  // When web sockets have errors, it isn't usually possible to identify
  // what went wrong. We make a request for the user's object to try
  // to identify the cause of the error
  webSocketErrorToaster(summary: string, _error: CloseEvent) {
    // Get user's own object to determine the auth error
    const authBody = JSON.parse(sessionStorage.getItem(AUTH_BODY));
    if (authBody != null && authBody.meta != null && authBody.meta.name != null) {
      this.publish(Eventtypes.FETCH_USER_OBJ, {
        success: () => {
          // Were able to get user obj, so we don't know what was wrong with the ws
          this.invokeRESTErrorToaster(summary, _error, false);
        },
        err: (error) => {
          if (error.statusCode === 400) {
            // This should never happen
            console.error('Controller.serverwebSocketErrorToaster() Failed to get user object when attempting to identify ws error');
            this.invokeRESTErrorToaster(summary, _error, false);
          } else {
            this.invokeRESTErrorToaster(summary, error, false);
          }
        }
      });
    }
  }

  webSocketErrorHandler(summary: string) {
    return (error) => {
      this.webSocketErrorToaster(summary, error);
    };
  }

  restErrorHandler(summary: string) {
    return (error) => {
      this.invokeRESTErrorToaster(summary, error);
    };
  }

  invokeErrorToaster(summary: string, errorMsg: string, buttons: ToolbarButton[] = [], shouldSplitLines: boolean = true) {
    if (shouldSplitLines) {
      errorMsg = errorMsg.split('. ').filter(x => x.length > 0).join('.\n');
    }
    this.messageService.add({
      severity: 'error',
      summary: summary,
      detail: errorMsg,
      sticky: true,
      buttons: buttons
    });
  }

  invokeConfirm(confirmMessage: Confirmation) {
    this.confirmService.confirm(confirmMessage);
  }
}
