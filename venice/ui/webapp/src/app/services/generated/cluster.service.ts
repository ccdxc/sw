import { HttpClient } from '@angular/common/http';
import { Injectable } from '@angular/core';
import { Eventtypes } from '@app/enum/eventtypes.enum';
import { VeniceResponse } from '@app/models/frontend/shared/veniceresponse.interface';
import { ControllerService } from '@app/services/controller.service';
import { ClusterDistributedServiceCard, ClusterHost, ClusterDSCProfile } from '@sdk/v1/models/generated/cluster';
import { UIRolePermissions } from '@sdk/v1/models/generated/UI-permissions-enum';
import { MethodOpts } from '@sdk/v1/services/generated/abstract.service';
import { Clusterv1Service } from '@sdk/v1/services/generated/clusterv1.service';
import { NEVER, Observable } from 'rxjs';
import { environment } from '../../../environments/environment';
import { Utility } from '../../common/Utility';
import { UIConfigsService } from '../uiconfigs.service';
import { GenServiceUtility } from './GenUtility';

@Injectable()
export class ClusterService extends Clusterv1Service {

  public  DATA_CACHE_TYPE_DSC = 'DistributedServiceCards';
  public  DATA_CACHE_TYPE_HOST = 'Hosts';
  public  DATA_CACHE_TYPE_NODE = 'Nodes';
  public  DATA_CACHE_TYPE_DSCPRPOFILE = 'DSCProfile';

  // Attributes used by generated services
  protected O_Tenant: string = this.getTenant();
  protected baseUrlAndPort = window.location.protocol + '//' + window.location.hostname + ':' + window.location.port;
  protected oboeServiceMap: { [method: string]: Observable<VeniceResponse> } = {};
  protected serviceUtility: GenServiceUtility;

  constructor(protected _http: HttpClient,
              protected _controllerService: ControllerService,
              protected uiconfigsService: UIConfigsService) {
      super(_http);
      this.serviceUtility = new GenServiceUtility(
        _http,
        (payload) => { this.publishAJAXStart(payload); },
        (payload) => { this.publishAJAXEnd(payload); }
      );
      this.serviceUtility.setId(this.getClassName());
      this.serviceUtility.createDataCache<ClusterDistributedServiceCard>(ClusterDistributedServiceCard, this.DATA_CACHE_TYPE_DSC, () => this.ListDistributedServiceCard(), (body: any) => this.WatchDistributedServiceCard(body));
      this.serviceUtility.createDataCache<ClusterHost>(ClusterHost, this.DATA_CACHE_TYPE_HOST, () => this.ListHost(), (body: any) => this.WatchHost(body));
      this.serviceUtility.createDataCache<ClusterDSCProfile>(ClusterDSCProfile, this.DATA_CACHE_TYPE_DSCPRPOFILE, () => this.ListDSCProfile(), (body: any) => this.WatchDSCProfile(body));

  }

  /**
   * Get the service class-name
   */
  getClassName(): string {
    return this.constructor.name;
  }

  public ListDistributedServiceCardCache() {
    return this.serviceUtility.handleListFromCache(this.DATA_CACHE_TYPE_DSC);
  }

  public ListHostCache() {
    return this.serviceUtility.handleListFromCache(this.DATA_CACHE_TYPE_HOST);
  }

  public ListDSCProfileCache() {
    return this.serviceUtility.handleListFromCache(this.DATA_CACHE_TYPE_DSCPRPOFILE);
  }


  protected publishAJAXStart(eventPayload: any) {
    this._controllerService.publish(Eventtypes.AJAX_START, eventPayload);
  }

  protected publishAJAXEnd(eventPayload: any) {
    this._controllerService.publish(Eventtypes.AJAX_END, eventPayload);
  }

  protected invokeAJAX(method: string, url: string, payload: any, opts: MethodOpts, forceReal: boolean = false): Observable<VeniceResponse> {

    const key = this.serviceUtility.convertEventID(opts);
    if (key !== UIRolePermissions.clusterversion_read && !this.uiconfigsService.isAuthorized(key)) {
      return NEVER;
    }
    const isOnline = !this.isToMockData() || forceReal;
    return this.serviceUtility.invokeAJAX(method, url, payload, opts.eventID, isOnline);
  }


  /**
   * Override-able api
   */
  public isToMockData(): boolean {
    const isUseRealData = this._controllerService.useRealData;
    return (!isUseRealData) ? isUseRealData : environment.isRESTAPIReady;
  }

  /**
   * Get login user tenant information
   */
  getTenant(): string {
    return Utility.getInstance().getTenant();
  }
}
