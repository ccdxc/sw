import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { TabcontentComponent } from './tabcontent.component';

describe('TabcontentComponent', () => {
  let component: TabcontentComponent;
  let fixture: ComponentFixture<TabcontentComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [TabcontentComponent]
    })
      .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(TabcontentComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
