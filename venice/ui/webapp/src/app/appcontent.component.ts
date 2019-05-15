import { OverlayContainer } from '@angular/cdk/overlay';
import { Component, HostBinding, OnDestroy, OnInit, ViewChild, ViewEncapsulation } from '@angular/core';
import { MatDialog } from '@angular/material';
import { MatSidenav, MatSidenavContainer } from '@angular/material/sidenav';
import { HttpEventUtility } from '@app/common/HttpEventUtility';
import { AUTH_BODY, logout } from '@app/core';
import { AlerttableService } from '@app/services/alerttable.service';
import { MonitoringService } from '@app/services/generated/monitoring.service';
import { LogService } from '@app/services/logging/log.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { IdleWarningComponent } from '@app/widgets/idlewarning/idlewarning.component';
import { ToolbarComponent } from '@app/widgets/toolbar/toolbar.component';
import { DEFAULT_INTERRUPTSOURCES, Idle } from '@ng-idle/core';
import { Store } from '@ngrx/store';
import { IApiStatus, IAuthUser } from '@sdk/v1/models/generated/auth';
import { ClusterVersion } from '@sdk/v1/models/generated/cluster';
import { MonitoringAlert } from '@sdk/v1/models/generated/monitoring';
import { Subject, Subscription } from 'rxjs';
import { map, takeUntil } from 'rxjs/operators';
import { CommonComponent } from './common.component';
import { Utility } from './common/Utility';
import { selectorSettings } from './components/settings-group';
import { Eventtypes } from './enum/eventtypes.enum';
import { ControllerService } from './services/controller.service';
import { AuthService } from './services/generated/auth.service';
import { ClusterService } from './services/generated/cluster.service';
import { sideNavMenu, SideNavItem } from './appcontent.sidenav';

export interface GetUserObjRequest {
  success: (resp: { body: IAuthUser | IApiStatus | Error; statusCode: number; }) => void;
  err: (err: IAuthUser | IApiStatus | Error) => void;
}

/**
 * This is the entry point component of Pensando-Venice Web-Application
 */
@Component({
  selector: 'app-content',
  templateUrl: './appcontent.component.html',
  styleUrls: ['./appcontent.component.scss', './appcontent.component.sidenav.scss', './appcontent.component.toast.scss'],
  encapsulation: ViewEncapsulation.None,
  providers: []
})
export class AppcontentComponent extends CommonComponent implements OnInit, OnDestroy {
  os = '';
  browsertype = '';
  browserversion = '';

  docLink: string = Utility.getDocURL();

  _currentComponent: any;

  // is Left-hand-side item click function registered?
  private _boolInitApp = false;

  protected isSideNavExpanded = true;

  subscriptions: Subscription[] = [];


  // idling
  showIdleWarning = false;
  idleDialogRef: any;

  // alerts related variables
  alertsEventUtility: HttpEventUtility<MonitoringAlert>;
  alertSubscription: Subscription;
  alerts: ReadonlyArray<MonitoringAlert> = [];
  alertNumbers = 0;

  alertQuery = {};

  sideNavMenu: SideNavItem[] = sideNavMenu;

  userName: string = '';

  versionEventUtility: HttpEventUtility<ClusterVersion>;
  versionArray: ReadonlyArray<ClusterVersion> = [];
  version: ClusterVersion = new ClusterVersion();

  @HostBinding('class') componentCssClass;
  private unsubscribeStore$: Subject<void> = new Subject<void>();
  @ViewChild('sidenav') _sidenav: MatSidenav;
  @ViewChild('rightSideNav') _rightSideNav: MatSidenav;
  @ViewChild('container') _container: MatSidenavContainer;
  @ViewChild('breadcrumbToolbar') _breadcrumbToolbar: ToolbarComponent;

  protected _rightSivNavIndicator = 'notifications';

  constructor(
    protected _controllerService: ControllerService,
    protected _logService: LogService,
    protected _alerttableSerivce: AlerttableService,
    public overlayContainer: OverlayContainer,
    private store: Store<any>,
    private idle: Idle,
    public dialog: MatDialog,
    protected uiconfigsService: UIConfigsService,
    protected monitoringService: MonitoringService,
    protected clusterService: ClusterService,
    protected authService: AuthService
  ) {
    super();
  }
  /**
   * Component life cycle event hook
   * It publishes event that AppComponent is about to instantiate
  */
  ngOnInit() {
    this.logger = Utility.getInstance().getLogService();

    this.os = Utility.getOperatingSystem();
    const browserObj = Utility.getBrowserInfomation();
    this.browsertype = browserObj['browserName'];
    this.browserversion = browserObj['browserName'] + browserObj['majorVersion'];
    this._initAppData();
    this.getVersion();

    this._subscribeToEvents();
    this._bindtoStore();
    const authBody = JSON.parse(sessionStorage.getItem(AUTH_BODY));
    if (authBody != null && authBody.meta != null && authBody.meta.name != null) {
      this.userName = authBody.meta.name;
    }
  }

  getVersion() {
    this.versionEventUtility = new HttpEventUtility<ClusterVersion>(ClusterVersion, true);
    this.versionArray = this.versionEventUtility.array as ReadonlyArray<ClusterVersion>;
    const subscription = this.clusterService.WatchVersion().subscribe(
      response => {
        this.versionEventUtility.processEvents(response);
        if (this.versionArray.length > 0) {
          this.version = this.versionArray[0];
        }
      },
    );
    this.subscriptions.push(subscription);
  }

  /**
   * Component life cycle event hook
   * It publishes event that AppComponent is about to exit
  */
  ngOnDestroy() {
    this.unsubscribeStore$.next();
    this.unsubscribeStore$.complete();
    this.unsubscribeAll();
    this._boolInitApp = false;
    this.idle.stop();
  }

  /**
   * Overide super's API
   * It will return AppComponent
   */
  getClassName(): string {
    return this.constructor.name;
  }

  private _bindtoStore() {
    this.store
      .select(selectorSettings)
      .pipe(
        takeUntil(this.unsubscribeStore$),
        map(({ theme }) => theme.toLowerCase())
      )
      .subscribe(theme => {
        this.componentCssClass = theme;
        this.overlayContainer.getContainerElement().classList.add(theme);
      });
  }

  /***** Routing *****/

  public navigate(paths: string[]) {
    this._controllerService.navigate(paths);
  }

  /**
   * handles case when user logout event occurs
   */
  private onLogout(payload: any) {
    this.store.dispatch(logout());
    this._boolInitApp = false;
    this.navigate(['/login']);
  }


  private onSearchResultReady(payload) {
    if (payload['id'] === 'searchresult') {
      this.navigate(['/searchresult']);
    } else {
      console.error(this.getClassName() + '.onSearchResultReady() payload.id not supported \n' + payload);
    }
  }


  private _subscribeToEvents() {
    this.subscriptions[Eventtypes.IDLE_CHANGE] = this._controllerService.subscribe(Eventtypes.IDLE_CHANGE, (payload) => {
      this.handleIdleChange(payload);
    });

    this.subscriptions[Eventtypes.LOGOUT] = this._controllerService.subscribe(Eventtypes.LOGOUT, (payload) => {
      this.onLogout(payload);
    });

    this.subscriptions[Eventtypes.COMPONENT_INIT] = this._controllerService.subscribe(Eventtypes.COMPONENT_INIT, (payload) => {
      this._handleComponentStateChangeInit(payload);
    });
    this.subscriptions[Eventtypes.COMPONENT_DESTROY] = this._controllerService.subscribe(Eventtypes.COMPONENT_DESTROY, (payload) => {
      this._handleComponentStateChangeDestroy(payload);
    });
    this.subscriptions[Eventtypes.SEARCH_RESULT_LOAD_REQUEST] = this._controllerService.subscribe(Eventtypes.SEARCH_RESULT_LOAD_REQUEST, (payload) => {
      this.onSearchResultReady(payload);
    });

    this.subscriptions[Eventtypes.FETCH_USER_OBJ] = this._controllerService.subscribe(Eventtypes.FETCH_USER_OBJ, (payload) => {
      this.getUserObj(payload);
    });
  }

  getUserObj(req: GetUserObjRequest) {
    this.authService.GetUser(this.userName).subscribe(req.success, req.err);
  }

  _handleComponentStateChangeInit(payload: any) {
    this._currentComponent = payload;
  }

  _initAppData() {
    if (this._boolInitApp === true) {
      return;
    } else {
      this.getAlerts();
    }
    this._boolInitApp = true;
    this._setupIdle();
  }

  _setupIdle() {
    this.idle.setIdle(this._controllerService.idleTime);
    this.idle.setTimeout(10);
    this.idle.setInterrupts(DEFAULT_INTERRUPTSOURCES);
    this.idle.onIdleEnd.subscribe(() => {
      if (this.idleDialogRef) {
        this.idleDialogRef.close();
      }
      this.showIdleWarning = false;
      this.idleDialogRef = null;
    });
    this.idle.onTimeout.subscribe(() => {
      if (this.idleDialogRef) {
        this.idleDialogRef.close();
      }
      this.showIdleWarning = false;
      this.idleDialogRef = null;
      this._controllerService.publish(Eventtypes.LOGOUT, { 'reason': 'Idle timeout' });
    });
    this.idle.onTimeoutWarning.subscribe((countdown) => {
      if (!this.showIdleWarning) {
        this.showIdleWarning = true;
        this.idleDialogRef = this.dialog.open(IdleWarningComponent, {
          width: '400px',
          hasBackdrop: true,
          data: { countdown: countdown }
        });
      } else {
        if (this.idleDialogRef != null && this.idleDialogRef.componentInstance != null) {
          this.idleDialogRef.componentInstance.updateCountdown(countdown);
        }
      }
    });
    if (this._controllerService.enableIdle) {
      this.idle.watch();
    }
  }

  handleIdleChange(payload: any) {
    if (payload.active != null) {
      if (payload.active) {
        this.idle.watch();
      } else {
        this.idle.stop();
      }
    }
    if (payload.time != null) {
      this.idle.setIdle(payload.time);
    }
  }

  _handleComponentStateChangeDestroy(payload: any) {
    this._currentComponent = null;
    if (this._breadcrumbToolbar) {
      this._breadcrumbToolbar.clear();
    }
  }

  /**
   * This function serves html template.
   * It transforms the SideNav panel to other state
   * @param event
   */
  onSidebarCollapseClick(event) {
    this.isSideNavExpanded = !this.isSideNavExpanded;
    const jQuery = Utility.getJQuery();
    if (jQuery('#app-sidebar').hasClass('pindown') !== true) {
      jQuery('#app-sidebar').toggleClass('active');
      if (event && event.currentTarget) {
        jQuery(event.currentTarget).toggleClass('active');

        // very important, it trigger the autosize of sidenav-container
        setTimeout(() => {
          this._sidenav.open();
        }, 500);
      }
    }
  }

  /**
   * This function serves html template.
   * It pins down the SideNav panel to a fixed position
   * @param
   */
  onSidebarPindownClick($event) {
    const jQuery = Utility.getJQuery();

    jQuery('#app-sidebar').toggleClass('pindown');
    jQuery('#app-sidebarCollapse').toggleClass('pindown');
  }

  onLogoutClick() {
    this._controllerService.publish(Eventtypes.LOGOUT, { 'reason': 'User logged out' });
  }

  /**
   * This API serves html template
   * It tells the right-sideNav which ng-template to use
   */
  whichRightSideNav() {
    return this._rightSivNavIndicator;
  }

  /**
   * This function serves html template.
   * @param  $event
   * @param id
   */
  onToolbarIconClick($event, id) {

    if (this._rightSivNavIndicator === id) {
      this._rightSideNav.toggle();
    } else {
      this._rightSivNavIndicator = id;
      this._rightSideNav.open();
    }
    setTimeout(() => {
      // programmatically trigger window resize to tell sideNav container adjust widow size
      window.dispatchEvent(new Event('resize'));
    }, 500);
  }

  /**
   * Call server to fetch all alerts to populate RHS alert-list
   */
  getAlerts() {
    this.alertsEventUtility = new HttpEventUtility<MonitoringAlert>(MonitoringAlert);
    if (this.alertSubscription) {
      this.alertSubscription.unsubscribe();
    }
    this.alertSubscription = this.monitoringService.WatchAlert(this.alertQuery).subscribe(
      response => {
        this.alertsEventUtility.processEvents(response);
        // this.alertQuery is empty. So we will get all alerts. We only need the alerts that are in open state. Alert table can update alerts. This will reflect the changes of alerts.
        this.alerts = this.alertsEventUtility.array.filter((alert: MonitoringAlert) => {
          return (this.isAlertInOpenState(alert));
        });
        // We are watching alerts. So when there are new alerts coming in, we display a toaster.
        if (this.alertNumbers > 0 && this.alertNumbers < this.alerts.length) {
          const diff = this.alerts.length - this.alertNumbers;
          const alertMsg = (diff === 1) ? diff + ' new alert arrived' : diff + 'new alerts arrived';
          this._controllerService.invokeInfoToaster('Alert', alertMsg);
        }
        this.alertNumbers = this.alerts.length;
      },
    );
    this.subscriptions.push(this.alertSubscription);
  }

  isAlertInOpenState(alert: MonitoringAlert): boolean {
    return (alert.spec.state === 'OPEN');
  }
  /**
   * This API serves html template
   * It response to user request of expanding all alerts (in RHS alert-list panel)
   * @param  alertlist
   */
  onExpandAllAlertsClick(alertlist) {
    this.navigate(['/monitoring', 'alertsevents']);
    this._rightSideNav.close();

  }

  settingsNavigate(route) {
    this._rightSideNav.close();
    this.navigate(route);
  }

  _componentSetup(component) {
    component.instance.style_class = 'wlWidget-positive';
    component.instance.color = '#7db1ea';
    component.instance.id = 'test_id';
    component.instance.title = 'Test';
    component.instance.label = '10';
    component.instance.styleType = 'positive';
    component.icon = {
      margin: {
        top: '5px',
        right: '5px',
        url: 'test.com'
      }
    };
    const dataset = {
      x: [1, 2, 3, 4, 5],
      y: [1, 3, 2, 3, 8]
    };

    component.instance.data = dataset;
    component.instance.updateData();
  }

}
