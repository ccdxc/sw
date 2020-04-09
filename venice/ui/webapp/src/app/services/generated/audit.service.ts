import {Injectable} from '@angular/core';
import {Auditv1Service} from '@sdk/v1/services/generated/auditv1.service';
import {Observable } from 'rxjs';
import {VeniceResponse} from '@app/models/frontend/shared/veniceresponse.interface';
import {GenServiceUtility} from '@app/services/generated/GenUtility';
import {HttpClient} from '@angular/common/http';
import {ControllerService} from '@app/services/controller.service';
import {Eventtypes} from '@app/enum/eventtypes.enum';
import {environment} from '@env/environment';
import {Utility} from '@common/Utility';
import { UIConfigsService } from '../uiconfigs.service';
import { UIRolePermissions } from '@sdk/v1/models/generated/UI-permissions-enum';
import { NEVER } from 'rxjs';
import { MethodOpts } from '@sdk/v1/services/generated/abstract.service';

@Injectable()
export class AuditService extends Auditv1Service {
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
  }


  /**
   * Get the service class-name
   */
  getClassName(): string {
    return this.constructor.name;
  }

  protected createDataCache<T>(constructor: any, key: string, listFn: () => Observable<VeniceResponse>, watchFn: (query: any) => Observable<VeniceResponse>) {
    return this.serviceUtility.createDataCache(constructor, key, listFn, watchFn);
  }

  protected getFromDataCache(kind: string, createCacheFn: any) {
    return this.serviceUtility.handleListFromCache(kind, createCacheFn);
  }

  protected publishAJAXStart(eventPayload: any) {
    this._controllerService.publish(Eventtypes.AJAX_START, eventPayload);
  }

  protected publishAJAXEnd(eventPayload: any) {
    this._controllerService.publish(Eventtypes.AJAX_END, eventPayload);
  }

  protected invokeAJAX(method: string, url: string, payload: any, opts: MethodOpts, forceReal: boolean = false): Observable<VeniceResponse> {

    const key = this.convertEventID(opts);
    if (!this.uiconfigsService.isAuthorized(key)) {
      return NEVER;
    }
    const isOnline = !this.isToMockData() || forceReal;
    return this.serviceUtility.invokeAJAX(method, url, payload, opts.eventID, isOnline);
  }

  convertEventID(opts) {
    // All event operations are reads, even posts.
    const key = 'auditevent' + '_' + 'read';
    return UIRolePermissions[key];
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
