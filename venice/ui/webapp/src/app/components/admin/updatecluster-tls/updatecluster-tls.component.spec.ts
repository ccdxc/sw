import { async, ComponentFixture, TestBed } from '@angular/core/testing';
import { Component } from '@angular/core';
import { configureTestSuite } from 'ng-bullet';
import { UpdateclusterTlsComponent } from './updatecluster-tls.component';
import { UpdateclustertlsComponent } from './updateclustertls/updateclustertls.component';
import { ClusterModule } from '@app/components/cluster-group/cluster/cluster.module';
import { SharedModule } from '@app/components/shared/shared.module';
import { MaterialdesignModule } from '@app/lib/materialdesign.module';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { RouterTestingModule } from '@angular/router/testing';
import {FormsModule, ReactiveFormsModule} from '@angular/forms';
import { ControllerService } from '@app/services/controller.service';
import { LogService } from '@app/services/logging/log.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { MessageService } from '@app/services/message.service';
import { ConfirmationService } from 'primeng/api';
import { AuthService } from '@app/services/auth.service';
import { HttpClientTestingModule } from '@angular/common/http/testing';
import { TestingUtility } from '@app/common/TestingUtility';



@Component({
  template: ''
})
class DummyComponent { }
 describe('UpdatecluterTlsComponent', () => {
  let component: UpdateclusterTlsComponent;
  let fixture: ComponentFixture<UpdateclusterTlsComponent>;
  let toolbarSpy: jasmine.Spy;
  configureTestSuite(() => {
    TestBed.configureTestingModule({
      declarations: [ UpdateclusterTlsComponent, UpdateclustertlsComponent, DummyComponent],
      imports: [
     ClusterModule,
     SharedModule,
     HttpClientTestingModule,
     FormsModule,
     ReactiveFormsModule,
     MaterialdesignModule,
     RouterTestingModule.withRoutes([{ path: 'login', component: DummyComponent }
      ])
      ],
      providers: [ UIConfigsService,
        ControllerService,
        LogService,
        LogPublishersService,
        MessageService,
        ConfirmationService,
        AuthService]
    });
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(UpdateclusterTlsComponent);
    component = fixture.componentInstance;
    const controllerService = TestBed.get(ControllerService);
    fixture.detectChanges();
    toolbarSpy = spyOn(controllerService, 'setToolbarData');
    TestingUtility.removeAllPermissions();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
  it('cert option no access', () => {
    fixture.detectChanges();
    expect(toolbarSpy.calls.argsFor(0).length).toBe(0);
  });
});
