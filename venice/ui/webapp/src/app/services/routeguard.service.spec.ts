import { HttpClientTestingModule } from '@angular/common/http/testing';
import { async, inject, TestBed } from '@angular/core/testing';
import { Router } from '@angular/router';
import { RouterTestingModule } from '@angular/router/testing';
import { ControllerService } from '@app/services/controller.service';
import { RouteGuard } from '@app/services/routeguard.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { MessageService } from 'primeng/primeng';

class MockUIConfigService extends UIConfigsService {
  static allowRoute = false;

  canActivateSubRoute(route: string): boolean {
    return MockUIConfigService.allowRoute;
  }
  navigateToHomepage() {
    this.router.navigate(['homepage']);
  }
}

describe('RouteGuardService', () => {
  let routeGuard: RouteGuard;
  let uiConfig: MockUIConfigService;

  // async beforeEach
  beforeEach(async(() => {
    TestBed.configureTestingModule({
      imports: [
        RouterTestingModule,
        HttpClientTestingModule
      ],
      providers: [RouteGuard,
        ControllerService,
        { provide: UIConfigsService, useClass: MockUIConfigService },
        MessageService
      ],
    })
      .compileComponents(); // compile template and css
  }));

  // synchronous beforeEach
  beforeEach(() => {
    routeGuard = TestBed.get(RouteGuard);
    uiConfig = TestBed.get(UIConfigsService);
  });

  it('check route guarding',
    // inject your guard service AND Router
    async(inject([
      Router,
    ],
      (
        router,
      ) => {
        // add a spy
        spyOn(router, 'navigate');
        spyOn(uiConfig, 'canActivateSubRoute').and.callThrough();
        const route: any = {
          pathFromRoot: [
            {
              url: [{ path: 'path1' }]
            },
            {
              url: [{ path: 'path2' }]
            },
            {
              url: [{ path: 'path3' }]
            }
          ]
        };
        expect(routeGuard.canActivateChild(route, null)).toBeFalsy();
        expect(uiConfig.canActivateSubRoute).toHaveBeenCalledWith('path1/path2/path3');
        expect(router.navigate).toHaveBeenCalledWith(['homepage']);

        MockUIConfigService.allowRoute = true;
        // Routing should be allowed now
        expect(routeGuard.canActivateChild(route, null)).toBeTruthy();
      })
    ));

});
