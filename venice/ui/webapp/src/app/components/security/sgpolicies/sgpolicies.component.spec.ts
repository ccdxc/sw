import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { SgpoliciesComponent } from './sgpolicies.component';
import { RouterTestingModule } from '@angular/router/testing';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { HttpClientTestingModule } from '@angular/common/http/testing';
import { SharedModule } from '@app/components/shared/shared.module';
import { RouterLinkStubDirective } from '@app/common/RouterLinkStub.directive';
import { ControllerService } from '@app/services/controller.service';
import { LogService } from '@app/services/logging/log.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { MatIconRegistry } from '@angular/material';
import { SecurityService } from '@app/services/generated/security.service';
import { PrimengModule } from '@app/lib/primeng.module';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { BehaviorSubject } from 'rxjs/BehaviorSubject';
import { SecuritySGPolicy } from '@sdk/v1/models/generated/security';
import { TestingUtility } from '@app/common/TestingUtility';
import { By } from '@angular/platform-browser';

describe('SgpoliciesComponent', () => {
  let component: SgpoliciesComponent;
  let fixture: ComponentFixture<SgpoliciesComponent>;
  let securityService;
  let sgPolicyObserver;
  let policy1;
  let policy2;
  let testingUtility: TestingUtility;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [SgpoliciesComponent, RouterLinkStubDirective],
      imports: [
        RouterTestingModule,
        NoopAnimationsModule,
        HttpClientTestingModule,
        SharedModule,
        PrimengModule,
      ],
      providers: [
        ControllerService,
        LogService,
        LogPublishersService,
        MatIconRegistry,
        SecurityService,
        UIConfigsService
      ]
    })
      .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(SgpoliciesComponent);
    component = fixture.componentInstance;
    securityService = TestBed.get(SecurityService);
    policy1 = new SecuritySGPolicy({
      meta: {
        name: 'policy1',
        'mod-time': '2018-08-23T17:35:08.534909931Z',
        'creation-time': '2018-08-23T17:30:08.534909931Z'
      }
    });
    policy2 = new SecuritySGPolicy({
      meta: {
        name: 'policy2',
        'mod-time': '2018-08-23T17:35:08.534909931Z',
        'creation-time': '2018-08-23T17:30:08.534909931Z'
      }
    });
    sgPolicyObserver = new BehaviorSubject({
      body: {
        result: {
          Events: [
            {
              Type: 'Created',
              Object: policy1
            }
          ]
        }
      }
    });
    spyOn(securityService, 'WatchSGPolicy').and.returnValue(
      sgPolicyObserver
    );
    testingUtility = new TestingUtility(fixture);
    fixture.detectChanges();
  });

  it('should list the policies in the table', () => {
    const tableElem = fixture.debugElement.query(By.css('.ui-table-scrollable-body-table tbody'));
    testingUtility.verifyTable([policy1], component.cols, tableElem, {});

    // Watch events
    sgPolicyObserver.next({
      body: {
        result: {
          Events: [
            {
              Type: 'Created',
              Object: policy2
            }
          ]
        }
      }
    });
    fixture.detectChanges();
    testingUtility.verifyTable([policy1, policy2], component.cols, tableElem, {});

    sgPolicyObserver.next({
      body: {
        result: {
          Events: [
            {
              Type: 'Deleted',
              Object: policy1
            }
          ]
        }
      }
    });
    fixture.detectChanges();
    testingUtility.verifyTable([policy2], component.cols, tableElem, {});

  });
});
