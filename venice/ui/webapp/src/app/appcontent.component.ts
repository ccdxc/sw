import { OverlayContainer } from '@angular/cdk/overlay';
import * as moment from 'moment';
import { Component, HostBinding, OnDestroy, OnInit, ViewChild, ViewEncapsulation, AfterViewInit } from '@angular/core';
import { MatDialog } from '@angular/material';
import { MatSidenav, MatSidenavContainer } from '@angular/material/sidenav';
import { HttpEventUtility } from '@app/common/HttpEventUtility';
import { logout } from '@app/core';
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
import { MonitoringAlert, IMonitoringAlert, IMonitoringAlertList } from '@sdk/v1/models/generated/monitoring';
import { Subject, Subscription, timer, interval } from 'rxjs';
import { map, takeUntil, tap, retryWhen, delayWhen } from 'rxjs/operators';
import { CommonComponent } from './common.component';
import { Utility } from './common/Utility';
import { selectorSettings } from './components/settings-group';
import { Eventtypes } from './enum/eventtypes.enum';
import { ControllerService } from './services/controller.service';
import { AuthService } from './services/generated/auth.service';
import { ClusterService } from './services/generated/cluster.service';
import { sideNavMenu, SideNavItem } from './appcontent.sidenav';
import { TemplatePortalDirective } from '@angular/cdk/portal';
import { UIRolePermissions } from '@sdk/v1/models/generated/UI-permissions-enum';
import { RolloutService } from '@app/services/generated/rollout.service';
import { RolloutRollout, RolloutRolloutStatus_state} from '@sdk/v1/models/generated/rollout';
import { ConfirmDialog } from 'primeng/primeng';

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
export class AppcontentComponent extends CommonComponent implements OnInit, OnDestroy, AfterViewInit {
  os = '';
  browsertype = '';
  browserversion = '';

  docLink: string = Utility.getDocURL();

  _currentComponent: any;

  // is Left-hand-side item click function registered?
  private _boolInitApp = false;

  protected isSideNavExpanded = true;

  subscriptions: Subscription[] = [];

  rolloutsEventUtility: HttpEventUtility<RolloutRollout>;
  rolloutObjects: ReadonlyArray<RolloutRollout>;
  currentRollout: RolloutRollout = null;

  // idling
  showIdleWarning = false;
  idleDialogRef: any;

  // alerts related variables
  alertsEventUtility: HttpEventUtility<MonitoringAlert>;
  alertGetSubscription: Subscription;
  alertWatchSubscription: Subscription;
  alerts: ReadonlyArray<MonitoringAlert> = [];
  startingAlertCount = 0;
  alertNumbers = 0;
  alertHighestSeverity: string;
  alertQuery = {};

  sideNavMenu: SideNavItem[] = sideNavMenu;

  userName: string = '';

  versionEventUtility: HttpEventUtility<ClusterVersion>;
  versionArray: ReadonlyArray<ClusterVersion> = [];
  version: ClusterVersion = new ClusterVersion();
  versionSubscription: Subscription = null;

  @HostBinding('class') componentCssClass;
  private unsubscribeStore$: Subject<void> = new Subject<void>();
  @ViewChild('sidenav') _sidenav: MatSidenav;
  @ViewChild('rightSideNav') _rightSideNav: MatSidenav;
  @ViewChild('container') _container: MatSidenavContainer;
  @ViewChild('breadcrumbToolbar') _breadcrumbToolbar: ToolbarComponent;
  @ViewChild('AppHelp') helpTemplate: TemplatePortalDirective;
  @ViewChild('confirmDialog') confirmDialog: ConfirmDialog;

  protected _rightSivNavIndicator = 'notifications';

  // isScreenBlocked generates an greyed-out overlay over the entire screen, prevents the user clicking anything else.
  // isUINavigationBlocked stops the sidenav and other elements from rendering.
  isScreenBlocked: boolean = false;
  isUINavigationBlocked: boolean = false;

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
    protected authService: AuthService,
    protected rolloutService: RolloutService,
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
    this.userName = Utility.getInstance().getLoginName() ;
    this.setMomentJSSettings();
  }

  updateRolloutUIBlock() {
    this.isUINavigationBlocked = true;
    if (this.uiconfigsService.isAuthorized(UIRolePermissions.rolloutrollout_read)) {
      this.getRollouts();
      this.isScreenBlocked = false;
    } else {
      this.isScreenBlocked = true;
      this._controllerService.invokeConfirm({
        header: 'Venice Unavailable',
        message: 'A rollout is in progress. Venice is temporarily unavailable.',
        acceptVisible: true,
        rejectVisible: false,
        accept: () => {
          // When a primeng alert is created, it tries to "focus" on a button, not adding a button returns an error.
          // So we create a button but hide it later.
        }
        });
      setTimeout(() => {
        this.confirmDialog.acceptVisible = false;
      }, 0);
    }
  }

  removeRolloutUIBlock() {
    this.isScreenBlocked = false;
    this.isUINavigationBlocked = false;
  }

  getRollouts() {
    this.rolloutsEventUtility = new HttpEventUtility<RolloutRollout>(RolloutRollout, true);
    this.rolloutObjects = this.rolloutsEventUtility.array as ReadonlyArray<RolloutRollout>;
    const subscription = this.rolloutService.WatchRollout().subscribe(
      response => {
        this.rolloutsEventUtility.processEvents(response);
        this.checkRollouts();
      },
      this._controllerService.restErrorHandler('Failed to get Rollouts info')
    );
    this.subscriptions.push(subscription);
  }

  checkRollouts() {
    for  (let i = 0; i < this.rolloutObjects.length; i++) {
      if (this.rolloutObjects[i].status.state === RolloutRolloutStatus_state.PROGRESSING) {
        this.currentRollout = this.rolloutObjects[i];
        break;
      } else if (this.rolloutObjects[i].status.state === RolloutRolloutStatus_state.SUCCESS && this.rolloutObjects[i] === this.currentRollout) {
        this.onRolloutComplete();
      }
    }
    Utility.getInstance().setCurrentRollout(this.currentRollout);
    this._controllerService.navigate(['/maintenance']);
  }

  setMomentJSSettings() {
    moment.updateLocale('en', {
      relativeTime : {
          future: 'in %s',
          past:   '%s ago',
          s  : 'one seconds',
          ss : '%d seconds',
          m:  'one minute',
          mm: '%d minutes',
          h:  'one hour',
          hh: '%d hours',
          d:  'one day',
          dd: '%d days',
          M:  'one month',
          MM: '%d months',
          y:  'one year',
          yy: '%d years'
      }
    });
    moment.relativeTimeThreshold('ss', 0);
    moment.relativeTimeThreshold('s', 60);
    moment.relativeTimeThreshold('m', 60);
    moment.relativeTimeThreshold('h', 24);
    moment.relativeTimeThreshold('d', 31);
    moment.relativeTimeThreshold('M', 12);
  }


  ngAfterViewInit() {
    // Example usage of setting help content
    // Once actual help data is in place
    // this should be removed.
    // TODO: Remove setting help template
    this._controllerService.setHelpOverlayData({
      template: this.helpTemplate
    });
  }

  redirectHome() {
    if (!this.isUINavigationBlocked) {
      this.uiconfigsService.navigateToHomepage();
    }
  }

  handleWatchVersionResponse(response) {
    this.versionEventUtility.processEvents(response);
    if (this.versionArray.length > 0) {
      this.version = this.versionArray[0];
      if (!!this.version.status['rollout-build-version']) {
        this.updateRolloutUIBlock();
        Utility.getInstance().setMaintenanceMode(true);
      } else {
        // set maintenance mode back to false when version object is reupdated.
        if (Utility.getInstance().getMaintenanceMode() === true) {
          Utility.getInstance().setMaintenanceMode(false);
          this.removeRolloutUIBlock();
        }
      }
    }
  }

  handleWatchVersionFail(error) {
    if (Utility.getInstance().getMaintenanceMode()) {
      this.getVersion();
    } else {
      this._controllerService.webSocketErrorToaster('Failed to get Cluster Version', error);
    }
  }

  getVersion() {
    this.versionEventUtility = new HttpEventUtility<ClusterVersion>(ClusterVersion, true);
    this.versionArray = this.versionEventUtility.array as ReadonlyArray<ClusterVersion>;
    if (this.versionSubscription) {
      this.versionSubscription.unsubscribe();
    } else {
      this.versionSubscription = this.clusterService.WatchVersion().subscribe(
        response => {
          this.handleWatchVersionResponse(response);
        },
        this.handleWatchVersionFail,
      );
      this.subscriptions.push(this.versionSubscription);
    }
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

  onRolloutComplete() {
    this.currentRollout = null;
    Utility.getInstance().setCurrentRollout(null);
    this._controllerService.invokeInfoToaster('Rollout', 'Rollout completed. Page will reload in 5 seconds.');
    setTimeout(() => {
      window.location.reload();
    }, 5000);
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

  updateHighestSeverity() {
    this.alertHighestSeverity = 'INFO';
    let alert: MonitoringAlert;
    for (let i = 0 ; i < this.alerts.length ; i++) {
      alert = this.alerts[i];
      if (alert.status.severity === 'CRITICAL') {
        this.alertHighestSeverity = 'CRITICAL';
        break;
      } else if (alert.status.severity === 'WARN' && this.alertHighestSeverity !== 'CRITICAL') {
        this.alertHighestSeverity = 'WARN';
      }
    }
  }

  getAlertNotificationClass() {
    if (this.alertHighestSeverity === 'CRITICAL') {
      return 'app-notification-critical';
    } else if (this.alertHighestSeverity === 'WARN') {
      return 'app-notification-warn';
    } else if (this.alertHighestSeverity === 'INFO') {
      return 'app-notification-info';
    }
  }
  /**
   * Call server to fetch all alerts to populate RHS alert-list
   */
  getAlerts() {
    this.alertsEventUtility = new HttpEventUtility<MonitoringAlert>(MonitoringAlert);
    if (this.alertGetSubscription) {
      this.alertGetSubscription.unsubscribe();
    }
    this.alertGetSubscription = this.monitoringService.ListAlert().subscribe(
      resp => {
        const body = resp.body as IMonitoringAlertList;
        if (body.items == null) {
          body.items = [];
        }
        this.startingAlertCount = body.items.length;
        this.alertNumbers = this.startingAlertCount;
        // Now that we have the count already in the system, we start the watch
        this.getAlertsWatch();
      },
      this._controllerService.webSocketErrorHandler('Failed to get Alerts'),
    );
    this.subscriptions.push(this.alertGetSubscription);
  }

  getAlertsWatch() {
    this.alertsEventUtility = new HttpEventUtility<MonitoringAlert>(MonitoringAlert);
    if (this.alertWatchSubscription) {
      this.alertWatchSubscription.unsubscribe();
    }
    this.alertWatchSubscription = this.monitoringService.WatchAlert(this.alertQuery).subscribe(
      response => {
        // NOTE: there is a max of 100 events that will come in a single chunk
        // The second chunk is NOT guaranteed to be
        this.alertsEventUtility.processEvents(response);
        // this.alertQuery is empty. So we will get all alerts. We only need the alerts that are in open state. Alert table can update alerts. This will reflect the changes of alerts.
        this.alerts = this.alertsEventUtility.array.filter((alert: MonitoringAlert) => {
          return (this.isAlertInOpenState(alert));
        });
        this.updateHighestSeverity();
        // We are watching alerts. So when there are new alerts coming in, we display a toaster.
        if (this.alertNumbers > 0 && this.alertNumbers < this.alerts.length) {
          const diff = this.alerts.length - this.alertNumbers;
          const alertMsg = (diff === 1) ? diff + ' new alert arrived' : diff + 'new alerts arrived';
          this._controllerService.invokeInfoToaster('Alert', alertMsg);
        }
        if (this.startingAlertCount == null) {
          this.alertNumbers = this.alerts.length;
        } else {
          this.alertNumbers = Math.max(this.startingAlertCount, this.alerts.length);
          if (this.alertNumbers === this.startingAlertCount) {
            // Have received all alerts that exist
            // no longer rely on startingAlertCount since we may receive delete events
            this.startingAlertCount = null;
          }
        }
      },
      this._controllerService.webSocketErrorHandler('Failed to get Alerts'),
    );
    this.subscriptions.push(this.alertWatchSubscription);
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
