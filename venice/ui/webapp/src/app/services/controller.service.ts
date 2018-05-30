import 'rxjs/add/operator/filter';
import 'rxjs/add/operator/map';

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
import { ToolbarData } from '@app/models/frontend/shared/toolbar.interface.ts';
import * as _ from 'lodash';
import { Promise } from 'q';
import { Subject } from 'rxjs/Subject';
import { Subscription } from 'rxjs/Subscription';

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
  private toolbarData: ToolbarData;
  private userSettings: any = {
    dashboardPinned: []
  };
  public useRealData = false;
  public _enableIdle = false;

  constructor(
    private _router: Router,
    private _injector: Injector,
    private _resolver: ComponentFactoryResolver,
    private _appRef: ApplicationRef,
    private loader: NgModuleFactoryLoader,
    protected domSanitizer: DomSanitizer,
    protected matIconRegistry: MatIconRegistry
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

    // network icons
    this.matIconRegistry.addSvgIcon('network-network', this.domSanitizer.bypassSecurityTrustResourceUrl('/assets/images/icons/network/ico-network-network.svg'));
    this.matIconRegistry.addSvgIcon('network-lbpolicy', this.domSanitizer.bypassSecurityTrustResourceUrl('/assets/images/icons/network/ico-network-loadbalance_policy.svg'));

    // security icons
    this.matIconRegistry.addSvgIcon('security-group', this.domSanitizer.bypassSecurityTrustResourceUrl('/assets/images/icons/security/ico-security-group.svg'));
    this.matIconRegistry.addSvgIcon('security-policy', this.domSanitizer.bypassSecurityTrustResourceUrl('/assets/images/icons/security/icon-security-policy.svg'));

    // Workload icons
    this.matIconRegistry.addSvgIcon('workloads-alert', this.domSanitizer.bypassSecurityTrustResourceUrl('/assets/images/icons/workload/icon-workloads-alert.svg'));
    this.matIconRegistry.addSvgIcon('workloads-new', this.domSanitizer.bypassSecurityTrustResourceUrl('/assets/images/icons/workload/icon-workloads-new.svg'));
    this.matIconRegistry.addSvgIcon('workloads-total', this.domSanitizer.bypassSecurityTrustResourceUrl('/assets/images/icons/workload/icon-workloads-total.svg'));
    this.matIconRegistry.addSvgIcon('workloads-unprotected', this.domSanitizer.bypassSecurityTrustResourceUrl('/assets/images/icons/workload/icon-workloads-unprotected.svg'));
    this.matIconRegistry.addSvgIcon('workload', this.domSanitizer.bypassSecurityTrustResourceUrl('/assets/images/icons/workload/icon-workload.svg'));
    this.matIconRegistry.addSvgIcon('workloads', this.domSanitizer.bypassSecurityTrustResourceUrl('/assets/images/icons/workload/icon-workloads.svg'));

    // Naple icons
    this.matIconRegistry.addSvgIcon('naples', this.domSanitizer.bypassSecurityTrustResourceUrl('/assets/images/icons/cluster/naples/ico-naples.svg'));


    this.matIconRegistry.addSvgIcon('un-pin', this.domSanitizer.bypassSecurityTrustResourceUrl('/assets/images/dashboard/ico-unpinned.svg'));

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
    return this.handler
      .filter(message => message.type === type)
      .map(message => message.payload)
      .subscribe(callback);
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
    return (this.LoginUserInfo && this.LoginUserInfo['kind'] !== undefined);
  }

  public directPageAsUserAlreadyLogin() {
    this._router.navigate(['dashboard']);
  }

  /***** Getting and setting toolbar data *****/
  public getToolbarData(): any {
    return this.toolbarData;
  }

  public setToolbarData(data: ToolbarData) {
    if (!_.isEqual(this.toolbarData, data)) {
      this.toolbarData = data;
      this.publish(Eventtypes.TOOLBAR_DATA_CHANGE, data);
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
}
