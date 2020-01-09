import { HttpClientTestingModule } from '@angular/common/http/testing';
import { Component, DebugElement } from '@angular/core';
import { ComponentFixture, TestBed } from '@angular/core/testing';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';
import { MatIconRegistry } from '@angular/material';
import { By } from '@angular/platform-browser';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { RouterTestingModule } from '@angular/router/testing';
import { configureTestSuite } from 'ng-bullet';
import { ConfirmationService } from 'primeng/api';
import { WidgetsModule } from 'web-app-framework';

import { TestingUtility } from '@app/common/TestingUtility';
import { SharedModule } from '@app/components/shared/shared.module';
import { MaterialdesignModule } from '@app/lib/materialdesign.module';
import { AuthService } from '@app/services/auth.service';
import { ControllerService } from '@app/services/controller.service';
import { ClusterService } from '@app/services/generated/cluster.service';
import { LogService } from '@app/services/logging/log.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { MessageService } from '@app/services/message.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { PrimengModule } from '@lib/primeng.module';
import { UpdateclusterTlsComponent } from './updatecluster-tls.component';

@Component({
  template: ''
})
class DummyComponent { }
 describe('UpdatecluterTlsComponent', () => {
  let component: UpdateclusterTlsComponent;
  let fixture: ComponentFixture<UpdateclusterTlsComponent>;
  let de: DebugElement;
  let testingUtility: TestingUtility;

  const keyCrappy = 'hello';
  const certsCrappy = 'world';

  configureTestSuite(() => {
    TestBed.configureTestingModule({
      declarations: [
        UpdateclusterTlsComponent,
        DummyComponent
      ],
      imports: [
        FormsModule,
        ReactiveFormsModule,
        NoopAnimationsModule,
        HttpClientTestingModule,
        PrimengModule,
        WidgetsModule,
        MaterialdesignModule,
        RouterTestingModule,
        SharedModule
      ],
      providers: [
        ControllerService,
        UIConfigsService,
        AuthService,
        ConfirmationService,
        LogService,
        LogPublishersService,
        MatIconRegistry,
        ClusterService,
        MessageService
      ]
    }).compileComponents();
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(UpdateclusterTlsComponent);
    component = fixture.componentInstance;
    de = fixture.debugElement;
    testingUtility = new TestingUtility(fixture);
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });

  it('should disable and enable upload correspondingly', () => {
    const keyTextArea = de.query(By.css('.updateclustertlsCert-key'));
    const certsTextArea = de.query(By.css('.updateclustertlsCert-certs'));

    // only one input
    testingUtility.setText(keyTextArea, keyCrappy);
    expect(component.isAllInputsValidated()).toBeFalsy();
    // two inputs, should enable upload button
    testingUtility.setText(certsTextArea, certsCrappy);
    expect(component.isAllInputsValidated()).toBeTruthy();

  });

  it('should have correct values after users input', () => {
    const keyTextArea = de.query(By.css('.updateclustertlsCert-key'));
    const certsTextArea = de.query(By.css('.updateclustertlsCert-certs'));
    testingUtility.setText(keyTextArea, keyCrappy);
    testingUtility.setText(certsTextArea, certsCrappy);

    expect(component.newCert.getFormGroupValues().certs).toBe(certsCrappy);
    expect(component.newCert.getFormGroupValues().key).toBe(keyCrappy);
  });
});
