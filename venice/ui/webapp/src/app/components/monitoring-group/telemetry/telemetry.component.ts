import { Component, OnDestroy, OnInit, ViewEncapsulation } from '@angular/core';
import { Animations } from '@app/animations';
import { Utility } from '@app/common/Utility';
import { BaseComponent } from '@app/components/base/base.component';
import { TimeRange } from '@app/components/shared/timerange/utility';
import { ChartItemType, GraphConfig, TelemetryPref, UserPreference } from '@app/models/frontend/shared/userpreference.interface';
import { ControllerService } from '@app/services/controller.service';
import { AuthService } from '@app/services/generated/auth.service';
import { ClusterService } from '@app/services/generated/cluster.service';
import { MetricsqueryService } from '@app/services/metricsquery.service';
import { AuthUserPreference, IAuthUserPreference } from '@sdk/v1/models/generated/auth';
import { Subscription } from 'rxjs';
import { TimeRangeOption, citadelTimeOptions, citadelMaxTimePeriod } from '@app/components/shared/timerange/timerange.component';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { UIRolePermissions } from '@sdk/v1/models/generated/UI-permissions-enum';

@Component({
  selector: 'app-telemetry',
  templateUrl: './telemetry.component.html',
  styleUrls: ['./telemetry.component.scss'],
  encapsulation: ViewEncapsulation.None,
  animations: [Animations]
})
export class TelemetryComponent extends BaseComponent implements OnInit, OnDestroy {
  userPref: UserPreference = {
    telemetry: {
      items: [],
      configuredGraphs: {}
    }
  };
  telemetryPref: TelemetryPref = {
    items: [],
    configuredGraphs: {}
  };
  preferenceLoaded: boolean = false;

  subscriptions: Subscription[] = [];

  // whether or not the user is in create/edit chart screen.
  inEditMode: boolean = false;
  // ID of the graph we are currently editing
  editGraphID: string = '';

  bodyIcon: any = {
    margin: {
      top: '9px',
      left: '8px',
    },
    matIcon: 'insert_chart_outlined'
  };


  saveUserPrefSub: Subscription;
  selectedTimeRange: TimeRange;

  timeRangeOptions: TimeRangeOption[] = citadelTimeOptions;
  maxTimePeriod = citadelMaxTimePeriod;

  constructor(protected controllerService: ControllerService,
    protected uiconfigsService: UIConfigsService,
    protected clusterService: ClusterService,
    protected authService: AuthService,
    protected telemetryqueryService: MetricsqueryService) {
      super(controllerService);
  }

  ngOnInit() {
    this.getUserPreferences();
    this.setToolbar();
  }

  setToolbar() {
    let buttons = [];
    if (this.uiconfigsService.isAuthorized(UIRolePermissions.authuserpreference_update)) {
      buttons = [
        {
          cssClass: 'global-button-primary telemetry-button',
          text: 'CREATE CHART',
          callback: () => { this.editChart(''); }
        },
      ];
    }
    this.controllerService.setToolbarData({
      buttons: buttons,
      breadcrumb: [{ label: 'Metrics', url: Utility.getBaseUIUrl() + 'monitoring/metrics' }]
    });
  }

  getUserPreferences() {
    this.authService.GetUserPreference(Utility.getInstance().getLoginName()).subscribe(
      response => {
        this.preferenceLoaded = true;
        const userPrefObj = response.body as IAuthUserPreference;
        // TODO: Validate the user preference object we are returned.
        if (userPrefObj.spec.options != null && userPrefObj.spec.options.length !== 0) {
          this.userPref = JSON.parse(userPrefObj.spec.options);
        }
        this.telemetryPref = this.userPref.telemetry;
      },
    );
  }

  editChart(graphID: string = null) {
    this.inEditMode = true;
    this.editGraphID = graphID;
  }

  onEditCancel() {
    this.getUserPreferences();
    this.inEditMode = false;
    this.editGraphID = '';
    this.setToolbar();
  }

  deleteChart(graphID: string) {
    const userPref = Utility.getLodash().cloneDeep(this.userPref);

    userPref.telemetry.items = userPref.telemetry.items.filter(x => x.graphID !== graphID);

    delete userPref.telemetry.configuredGraphs[graphID];

    const onSuccess = (resp) => {
      this.controllerService.invokeSuccessToaster(Utility.DELETE_SUCCESS_SUMMARY, 'Chart deleted');
      this.onEditCancel();
    };
    const onError = (error) => {
      this._controllerService.invokeRESTErrorToaster(Utility.DELETE_FAILED_SUMMARY, error);
    };

    this.saveUserPreference(userPref, onSuccess, onError);
  }

  onChartSave(config: GraphConfig) {
    let isUpdate = true;
    if (config.id == null || config.id.length === 0) {
      isUpdate = false;
      // pick an id for the chart
      let id = Utility.s4();
      while (this.telemetryPref.configuredGraphs[id] != null) {
        // Make sure the id is random
        id = Utility.s4();
      }
      config.id = id;
    }
    const userPref = Utility.getLodash().cloneDeep(this.userPref);
    userPref.telemetry.configuredGraphs[config.id] = config;
    if (!isUpdate) {

    userPref.telemetry.items.push({
      type: ChartItemType.chart,
      graphID: config.id
    });
    }
    const onSuccess = (resp) => {
      if (isUpdate) {
        this.controllerService.invokeSuccessToaster(Utility.CREATE_SUCCESS_SUMMARY, 'Chart updated');
      } else {
        this.controllerService.invokeSuccessToaster(Utility.CREATE_SUCCESS_SUMMARY, 'Chart created');
      }
      this.onEditCancel();
    };
    const onError = (error) => {
      this._controllerService.invokeRESTErrorToaster(Utility.CREATE_FAILED_SUMMARY, error);
    };
    this.saveUserPreference(userPref, onSuccess, onError);
  }

  saveUserPreference(userPref: UserPreference, onSuccess, onError) {
    const authUserPref = new AuthUserPreference;
    authUserPref.spec.options = JSON.stringify(userPref);
    if (this.saveUserPrefSub != null) {
      this.saveUserPrefSub.unsubscribe();
    }
    this.saveUserPrefSub = this.authService.UpdateUserPreference(Utility.getInstance().getLoginName(), authUserPref).subscribe({
      next: onSuccess,
      error: onError
    });
    this.subscriptions.push(this.saveUserPrefSub);
  }

  setTimeRange(timeRange: TimeRange) {
    // Pushing into next event loop
    setTimeout(() => {
      this.selectedTimeRange = timeRange;
    }, 0);
  }

  ngOnDestroy() {
    this.subscriptions.forEach(subscription => {
      subscription.unsubscribe();
    });
  }

}
