// SPDX-License-Identifier: GPL-2.0-only
// Generated with linux-mdss-dsi-panel-driver-generator from vendor device tree:
//   Copyright (c) 2013, The Linux Foundation. All rights reserved.
// Published in Xo666/mainline-instantnoodle, commit 417fa473cdc7.

#include <linux/backlight.h>
#include <linux/debugfs.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/gpio/consumer.h>
#include <linux/mod_devicetable.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/property.h>
#include <linux/regulator/consumer.h>
#include <linux/seq_file.h>

#include <video/mipi_display.h>

#include <drm/display/drm_dsc.h>
#include <drm/display/drm_dsc_helper.h>
#include <drm/drm_connector.h>
#include <drm/drm_crtc.h>
#include <drm/drm_mipi_dsi.h>
#include <drm/drm_modes.h>
#include <drm/drm_panel.h>
#include <drm/drm_probe_helper.h>

struct samsung_amb655uv01 {
	struct drm_panel panel;
	struct drm_connector *connector;
	struct mipi_dsi_device *dsi;
	struct drm_dsc_config dsc;
	bool first_prepare;
	bool keep_dsc_on_blank;
	bool awake;
	unsigned int wake_count, blank_count;
	struct mutex transfer_lock;
	struct gpio_desc *reset_gpio;
	struct regulator_bulk_data supplies[5];
	unsigned int num_supplies;
};

static inline
struct samsung_amb655uv01 *to_samsung_amb655uv01(struct drm_panel *panel)
{
	return container_of(panel, struct samsung_amb655uv01, panel);
}

static void samsung_amb655uv01_write(struct mipi_dsi_multi_context *ctx,
				   const void *data, size_t len)
{
	struct mipi_dsi_device *dsi = ctx->dsi;
	const struct mipi_dsi_host_ops *ops = dsi->host->ops;
	struct mipi_dsi_msg msg = {
		.channel = dsi->channel,
		.type = MIPI_DSI_DCS_LONG_WRITE,
		.tx_buf = data,
		.tx_len = len,
	};
	ssize_t ret;

	if (ctx->accum_err)
		return;
	if (!ops || !ops->transfer) {
		ctx->accum_err = -EOPNOTSUPP;
		return;
	}

	/* The factory sequence uses long packets even for two-byte writes. */
	if (dsi->mode_flags & MIPI_DSI_MODE_LPM)
		msg.flags |= MIPI_DSI_MSG_USE_LPM;
	ret = ops->transfer(dsi->host, &msg);
	if (ret < 0) {
		ctx->accum_err = ret;
		dev_err(&dsi->dev, "Failed to write panel command: %zd\n", ret);
	}
}

#define samsung_amb655uv01_write_seq(ctx, seq...) do { \
	static const u8 data[] = { seq }; \
	samsung_amb655uv01_write(ctx, data, ARRAY_SIZE(data)); \
} while (0)

static void samsung_amb655uv01_reset(struct samsung_amb655uv01 *ctx)
{
	gpiod_set_value_cansleep(ctx->reset_gpio, 0);
	usleep_range(10000, 11000);
	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	usleep_range(1000, 2000);
	gpiod_set_value_cansleep(ctx->reset_gpio, 0);
	usleep_range(10000, 11000);
}

static int samsung_amb655uv01_on(struct samsung_amb655uv01 *ctx)
{
	struct mipi_dsi_multi_context dsi_ctx = { .dsi = ctx->dsi };

	ctx->dsi->mode_flags |= MIPI_DSI_MODE_LPM;

	samsung_amb655uv01_write_seq(&dsi_ctx, 0xf0, 0x5a, 0x5a);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0x9d, 0x01);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0x9e,
				     0x11, 0x00, 0x00, 0x89, 0x30, 0x80, 0x09,
				     0x60, 0x04, 0x38, 0x00, 0x30, 0x02, 0x1c,
				     0x02, 0x1c, 0x02, 0x00, 0x02, 0x52, 0x00,
				     0x20, 0x04, 0x23, 0x00, 0x07, 0x00, 0x0f,
				     0x02, 0x8e, 0x02, 0x1f, 0x18, 0x00, 0x10,
				     0xf0, 0x03, 0x0c, 0x20, 0x00, 0x06, 0x0b,
				     0x0b, 0x33, 0x0e, 0x1c, 0x2a, 0x38, 0x46,
				     0x54, 0x62, 0x69, 0x70, 0x77, 0x79, 0x7b,
				     0x7d, 0x7e, 0x01, 0x02, 0x01, 0x00, 0x09,
				     0x40, 0x09, 0xbe, 0x19, 0xfc, 0x19, 0xfa,
				     0x19, 0xf8, 0x1a, 0x38, 0x1a, 0x78, 0x1a,
				     0xb6, 0x2a, 0xb6, 0x2a, 0xf4, 0x2a, 0xf4,
				     0x4b, 0x34, 0x63, 0x74);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xf0, 0xa5, 0xa5); //KEY DISABLE
	mipi_dsi_dcs_exit_sleep_mode_multi(&dsi_ctx);
	mipi_dsi_msleep(&dsi_ctx, 20);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xf0, 0x5a, 0x5a); //KEY ENABLE
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xf5, 0x85);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xb0, 0x01);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xf5, 0x77);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xb0, 0x03);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xf5, 0x10, 0x40, 0x10, 0x40);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xf0, 0xa5, 0xa5);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xf0, 0x5a, 0x5a);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xb0, 0x3b);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xf5, 0x25);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xf0, 0xa5, 0xa5);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xf0, 0x5a, 0x5a);
	samsung_amb655uv01_write_seq(&dsi_ctx, MIPI_DCS_SET_TEAR_ON, 0x00);
	/* Disable the command page for the following address writes. */
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xf0, 0xa5, 0xa5);
	mipi_dsi_dcs_set_column_address_multi(&dsi_ctx, 0x0000, 0x0437);
	mipi_dsi_dcs_set_page_address_multi(&dsi_ctx, 0x0000, 0x095f);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xf0, 0x5a, 0x5a); //ENABLE
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xb0, 0x05);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xd7, 0x07, 0x02, 0x40);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xf0, 0xa5, 0xa5);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xf0, 0x5a, 0x5a);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xf5, 0x85);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xb0, 0x02);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xf8, 0xc4);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xb0, 0x04);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xf8,
				     0x41, 0x47, 0x11, 0xb8, 0xb8);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xb0, 0x02);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xf8, 0xcc);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xf0, 0xa5, 0xa5);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xf0, 0x5a, 0x5a);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xb0, 0x08);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xf2, 0xa0);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xf0, 0xa5, 0xa5); //DISABLED
	mipi_dsi_msleep(&dsi_ctx, 110); //Sleep for 110ms
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xf0, 0x5a, 0x5a); //ENABLED
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xf5, 0x87);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xf0, 0xa5, 0xa5);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xf0, 0x5a, 0x5a);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xb0, 0x06);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xb7, 0x01);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xb0, 0x05);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xb7, 0x13);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xb7, 0x01, 0x37);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xf0, 0xa5, 0xa5); //DISABLED
	samsung_amb655uv01_write_seq(&dsi_ctx, MIPI_DCS_WRITE_CONTROL_DISPLAY,
				     0x20);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xf0, 0x5a, 0x5a); //RE_ENABLED
	samsung_amb655uv01_write_seq(&dsi_ctx, 0x60, 0x10);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xf0, 0xa5, 0xa5);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xf0, 0x5a, 0x5a);
	samsung_amb655uv01_write_seq(&dsi_ctx, MIPI_DCS_WRITE_POWER_SAVE, 0x00);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xf0, 0xa5, 0xa5); //Disable Key
	mipi_dsi_dcs_set_display_on_multi(&dsi_ctx); //Turn on screen

	return dsi_ctx.accum_err;
}

static int samsung_amb655uv01_off(struct samsung_amb655uv01 *ctx)
{
	struct mipi_dsi_multi_context dsi_ctx = { .dsi = ctx->dsi };

	ctx->dsi->mode_flags &= ~MIPI_DSI_MODE_LPM;

	mipi_dsi_dcs_set_display_off_multi(&dsi_ctx);
	mipi_dsi_usleep_range(&dsi_ctx, 10000, 11000);
	mipi_dsi_dcs_enter_sleep_mode_multi(&dsi_ctx);
	mipi_dsi_msleep(&dsi_ctx, 120);

	return dsi_ctx.accum_err;
}

/* Select the DDIC refresh together with the host mode. The panel's
 * timing-switch packets are byte-for-byte from the handset ANA6705 DT.
 * Atomic enable runs after the connector/CRTC states have been swapped.
 */
static int samsung_amb655uv01_set_refresh(struct samsung_amb655uv01 *ctx)
{
	struct mipi_dsi_multi_context dsi_ctx = { .dsi = ctx->dsi };
	unsigned long flags = ctx->dsi->mode_flags;
	unsigned int hz = 90;

	if (ctx->connector && ctx->connector->state &&
	    ctx->connector->state->crtc && ctx->connector->state->crtc->state) {
		const struct drm_display_mode *mode =
			&ctx->connector->state->crtc->state->adjusted_mode;
		if (mode->clock)
			hz = drm_mode_vrefresh(mode) < 75 ? 60 : 90;
	}
	ctx->dsi->mode_flags &= ~MIPI_DSI_MODE_LPM;
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xf0, 0x5a, 0x5a);
	if (hz == 90)
		samsung_amb655uv01_write_seq(&dsi_ctx, 0x60, 0x10);
	else
		samsung_amb655uv01_write_seq(&dsi_ctx, 0x60, 0x00);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xf0, 0xa5, 0xa5);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xf0, 0x5a, 0x5a);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xb0, 0x69);
	if (hz == 90)
		samsung_amb655uv01_write_seq(&dsi_ctx, 0xe6, 0x00);
	else
		samsung_amb655uv01_write_seq(&dsi_ctx, 0xe6, 0x02);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xb0, 0x77);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xe6, 0xb3);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xf0, 0xa5, 0xa5);
	ctx->dsi->mode_flags = flags;
	if (!dsi_ctx.accum_err)
		dev_info(&ctx->dsi->dev, "stock panel refresh=%u link-target=830000000\n", hz);
	return dsi_ctx.accum_err;
}

static int samsung_amb655uv01_prepare(struct drm_panel *panel)
{
	struct samsung_amb655uv01 *ctx = to_samsung_amb655uv01(panel);
	struct device *dev = &ctx->dsi->dev;
	struct drm_dsc_picture_parameter_set pps;
	bool warm = ctx->keep_dsc_on_blank && !ctx->first_prepare;
	int ret;

	guard(mutex)(&ctx->transfer_lock);
	if (!warm) {
		samsung_amb655uv01_reset(ctx);

		ret = samsung_amb655uv01_on(ctx);
		if (ret < 0) {
			dev_err(dev, "Failed to initialize panel: %d\n", ret);
			gpiod_set_value_cansleep(ctx->reset_gpio, 1);
			return ret;
		}
	}

	drm_dsc_pps_payload_pack(&pps, &ctx->dsc);

	ret = mipi_dsi_picture_parameter_set(ctx->dsi, &pps);
	if (ret < 0) {
		dev_err(panel->dev, "failed to transmit PPS: %d\n", ret);
		return ret;
	}

	ret = mipi_dsi_compression_mode(ctx->dsi, true);
	if (ret < 0) {
		dev_err(dev, "failed to enable compression mode: %d\n", ret);
		return ret;
	}

	msleep(28); /* TODO: Is this panel-dependent? */
	if (warm) {
		ctx->dsi->mode_flags |= MIPI_DSI_MODE_LPM;
		ret = mipi_dsi_dcs_set_display_on(ctx->dsi);
		if (ret < 0)
			return ret;
		msleep(20);
	}
	ret = samsung_amb655uv01_set_refresh(ctx);
	if (ret < 0)
		return ret;
	ctx->first_prepare = false;
	ctx->awake = true;
	ctx->wake_count++;
	dev_info(dev, "panel wake=%u warm=%d DSC/PPS restored\n", ctx->wake_count, warm);

	return 0;
}

static int samsung_amb655uv01_unprepare(struct drm_panel *panel)
{
	struct samsung_amb655uv01 *ctx = to_samsung_amb655uv01(panel);
	struct device *dev = &ctx->dsi->dev;
	int ret;

	guard(mutex)(&ctx->transfer_lock);
	if (ctx->keep_dsc_on_blank && !ctx->first_prepare) {
		/* Keep the initialized DSC receiver across DPMS. Deep sleep 10h
		 * plus a reset without a real AVDD power cycle can leave this
		 * handset's panel dark even though the host scans out frames.
		 */
		ctx->dsi->mode_flags |= MIPI_DSI_MODE_LPM;
		ret = mipi_dsi_dcs_set_display_off(ctx->dsi);
		if (ret < 0)
			return ret;
		msleep(20);
		ctx->awake = false;
		ctx->blank_count++;
		dev_info(dev, "panel blank=%u DSC retained; no deep sleep/reset\n",
			 ctx->blank_count);
		return 0;
	}
	ret = samsung_amb655uv01_off(ctx);
	if (ret < 0)
		dev_err(dev, "Failed to un-initialize panel: %d\n", ret);

	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	ctx->awake = false;

	return 0;
}

static const struct drm_display_mode samsung_amb655uv01_modes[] = {
	{ /* 90Hz Mode */
		/* In command mode, horizontal blanking expresses transfer
		 * overhead. With DSC, msm replaces 1080 active pixels by 360.
		 * 636 * 2416 * 90 * 24 / 4 = 829751040 bit/s per lane,
		 * within 0.03% of this handset's stock 830000000 bit/s.
		 */
		.clock = (1080 + 52 + 24 + 200) * (2400 + 4 + 4 + 8) * 90 / 1000,
		.hdisplay = 1080,
		.hsync_start = 1080 + 52,
		.hsync_end = 1080 + 52 + 24,
		.htotal = 1080 + 52 + 24 + 200,
		.vdisplay = 2400,
		.vsync_start = 2400 + 4,
		.vsync_end = 2400 + 4 + 4,
		.vtotal = 2400 + 4 + 4 + 8,
		.width_mm = 70,
		.height_mm = 151,
		.type = DRM_MODE_TYPE_DRIVER,
	},
	{
		/* 954 compressed horizontal periods keeps the same link at60Hz. */
		.clock = (1080 + 52 + 24 + 518) * (2400 + 4 + 4 + 8) * 60 / 1000,
		.hdisplay = 1080,
		.hsync_start = 1080 + 52,
		.hsync_end = 1080 + 52 + 24,
		.htotal = 1080 + 52 + 24 + 518,
		.vdisplay = 2400,
		.vsync_start = 2400 + 4,
		.vsync_end = 2400 + 4 + 4,
		.vtotal = 2400 + 4 + 4 + 8,
		.width_mm = 70,
		.height_mm = 151,
		.type = DRM_MODE_TYPE_DRIVER,
	},
};

static int samsung_amb655uv01_get_modes(struct drm_panel *panel,
				      struct drm_connector *connector)
{
	struct samsung_amb655uv01 *ctx = to_samsung_amb655uv01(panel);
	int count = 0;

	ctx->connector = connector;
	for (int i = 0; i < ARRAY_SIZE(samsung_amb655uv01_modes); i++) {
		count += drm_connector_helper_get_modes_fixed(connector,
						    &samsung_amb655uv01_modes[i]);
	}

	return count;
}

static int samsung_amb655uv01_status_show(struct seq_file *s, void *data)
{
	struct drm_panel *panel = s->private;
	struct samsung_amb655uv01 *ctx = to_samsung_amb655uv01(panel);

	guard(mutex)(&ctx->transfer_lock);

	seq_printf(s, "initialized=%u awake=%u wakes=%u blanks=%u\n",
		   !ctx->first_prepare, ctx->awake, ctx->wake_count,
		   ctx->blank_count);
	/* Live DCS readback can disturb the command-mode scanout. */
	seq_printf(s, "brightness=%u (cached)\n",
		   panel->backlight->props.brightness);

	return 0;
}
DEFINE_SHOW_ATTRIBUTE(samsung_amb655uv01_status);

static void samsung_amb655uv01_debugfs_init(struct drm_panel *panel,
					  struct dentry *root)
{
	debugfs_create_file("panel_status", 0400, root, panel,
			    &samsung_amb655uv01_status_fops);
}

static const struct drm_panel_funcs samsung_amb655uv01_panel_funcs = {
	.prepare = samsung_amb655uv01_prepare,
	.unprepare = samsung_amb655uv01_unprepare,
	.get_modes = samsung_amb655uv01_get_modes,
	.debugfs_init = samsung_amb655uv01_debugfs_init,
};

static int samsung_amb655uv01_bl_update_status(struct backlight_device *bl)
{
	struct mipi_dsi_device *dsi = bl_get_data(bl);
	struct samsung_amb655uv01 *ctx = mipi_dsi_get_drvdata(dsi);
	unsigned long mode_flags;
	u16 brightness;
	int ret;

	guard(mutex)(&ctx->transfer_lock);
	mode_flags = dsi->mode_flags;
	brightness = backlight_get_brightness(bl);
	dsi->mode_flags &= ~MIPI_DSI_MODE_LPM;
	ret = mipi_dsi_dcs_set_display_brightness_large(dsi, brightness);
	/* A failed transfer must not change the next command's link mode. */
	dsi->mode_flags = mode_flags;

	return ret;
}

static const struct backlight_ops samsung_amb655uv01_bl_ops = {
	.update_status = samsung_amb655uv01_bl_update_status,
	/* Use the backlight core's cached brightness. DCS 0x52 readback is
	 * unreliable on this panel and can leave the DSI command path wedged.
	 */
};

static struct backlight_device *
samsung_amb655uv01_create_backlight(struct mipi_dsi_device *dsi)
{
	struct device *dev = &dsi->dev;
	const struct backlight_properties props = {
		.type = BACKLIGHT_RAW,
		.brightness = 1048,
		.max_brightness = 1740, //15% reduction to prevent burn-in, original value is 2047
	};

	return devm_backlight_device_register(dev, dev_name(dev), dev, dsi,
					      &samsung_amb655uv01_bl_ops, &props);
}

static int samsung_amb655uv01_probe(struct mipi_dsi_device *dsi)
{
	struct device *dev = &dsi->dev;
	struct samsung_amb655uv01 *ctx;
	int ret;


	ctx = devm_drm_panel_alloc(dev, struct samsung_amb655uv01, panel,
				   &samsung_amb655uv01_panel_funcs,
				   DRM_MODE_CONNECTOR_DSI);
	if (IS_ERR(ctx))
		return PTR_ERR(ctx);

	ctx->supplies[0].supply = "vddio";
	ctx->supplies[1].supply = "vdd";
	ctx->supplies[2].supply = "avdd";
	ctx->num_supplies = 3;
	if (device_property_present(dev, "vddd-supply")) {
		ctx->supplies[3].supply = "vddd";
		ctx->num_supplies = 4;
	}

	if (device_property_present(dev, "vddr-supply"))
		ctx->supplies[ctx->num_supplies++].supply = "vddr";

	ctx->reset_gpio = devm_gpiod_get(dev, "reset", GPIOD_OUT_HIGH);
	if (IS_ERR(ctx->reset_gpio))
		return dev_err_probe(dev, PTR_ERR(ctx->reset_gpio),
				     "Failed to get reset GPIO\n");

	ret = devm_regulator_bulk_get(dev, ctx->num_supplies, ctx->supplies);
	if (ret)
		return dev_err_probe(dev, ret,
				     "Failed to get vddio regulator\n");

	if (device_property_present(dev, "vddr-supply")) {
		/* Handset stock ANA6705 votes: VDDI1.8V/62mA,
		 * VCI3.0V/857mA and VDDR1.3V/62mA. Small voltage
		 * ranges permit the PMIC's8mV selector rounding.
		 */
		ret = regulator_set_voltage(ctx->supplies[0].consumer, 1800000, 1808000);
		if (ret)
			return dev_err_probe(dev, ret, "VDDI voltage vote failed\n");
		ret = regulator_set_load(ctx->supplies[0].consumer, 62000);
		if (ret)
			return dev_err_probe(dev, ret, "VDDI load vote failed\n");
		ret = regulator_set_voltage(ctx->supplies[1].consumer, 3000000, 3008000);
		if (ret)
			return dev_err_probe(dev, ret, "VCI voltage vote failed\n");
		ret = regulator_set_load(ctx->supplies[1].consumer, 857000);
		if (ret)
			return dev_err_probe(dev, ret, "VCI load vote failed\n");
		ret = regulator_set_voltage(ctx->supplies[ctx->num_supplies - 1].consumer,
					    1300000, 1304000);
		if (ret)
			return dev_err_probe(dev, ret, "VDDR voltage vote failed\n");
		ret = regulator_set_load(ctx->supplies[ctx->num_supplies - 1].consumer, 62000);
		if (ret)
			return dev_err_probe(dev, ret, "VDDR load vote failed\n");
	}
	ret = regulator_bulk_enable(ctx->num_supplies, ctx->supplies);
	if (ret < 0)
		return dev_err_probe(dev, ret,
				     "Failed to enable regulators\n");

	ctx->first_prepare = true;
	ctx->keep_dsc_on_blank = device_property_read_bool(dev, "oneplus,keep-dsc-on-blank");
	mutex_init(&ctx->transfer_lock);

	ctx->dsi = dsi;
	mipi_dsi_set_drvdata(dsi, ctx);

	dsi->lanes = 4;
	dsi->format = MIPI_DSI_FMT_RGB888;
	dsi->mode_flags = MIPI_DSI_MODE_DSC_ALL_SLICES_IN_PKT;


	ctx->panel.backlight = samsung_amb655uv01_create_backlight(dsi);
	if (IS_ERR(ctx->panel.backlight))
		return dev_err_probe(dev, PTR_ERR(ctx->panel.backlight),
				     "Failed to create backlight\n");
	ctx->panel.prepare_prev_first = true;

	drm_panel_add(&ctx->panel);

	/* This panel only supports DSC; unconditionally enable it */
	dsi->dsc = &ctx->dsc;

	ctx->dsc.dsc_version_major = 1;
	ctx->dsc.dsc_version_minor = 1;

	/* Both 540-pixel slices of each line travel in one DSI packet. */
	ctx->dsc.slice_height = 48;
	ctx->dsc.slice_width = 540;
	/*
	 * TODO: hdisplay should be read from the selected mode once
	 * it is passed back to drm_panel (in prepare?)
	 */
	WARN_ON(1080 % ctx->dsc.slice_width);
	ctx->dsc.slice_count = 1080 / ctx->dsc.slice_width;
	ctx->dsc.bits_per_component = 8;
	ctx->dsc.bits_per_pixel = 8 << 4; /* 4 fractional bits */
	ctx->dsc.block_pred_enable = true;

	ret = mipi_dsi_attach(dsi);
	if (ret < 0) {
		dev_err(dev, "Failed to attach to DSI host: %d\n", ret);
		drm_panel_remove(&ctx->panel);
		return ret;
	}

	return 0;
}

static void samsung_amb655uv01_remove(struct mipi_dsi_device *dsi)
{
	struct samsung_amb655uv01 *ctx = mipi_dsi_get_drvdata(dsi);
	int ret;

	ret = mipi_dsi_detach(dsi);
	if (ret < 0)
		dev_err(&dsi->dev, "Failed to detach from DSI host: %d\n", ret);

	drm_panel_remove(&ctx->panel);
}

static const struct of_device_id samsung_amb655uv01_of_match[] = {
	{ .compatible = "samsung,amb655uv01" },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, samsung_amb655uv01_of_match);

static struct mipi_dsi_driver samsung_amb655uv01_driver = {
	.probe = samsung_amb655uv01_probe,
	.remove = samsung_amb655uv01_remove,
	.driver = {
		.name = "panel-samsung-amb655uv01",
		.of_match_table = samsung_amb655uv01_of_match,
	},
};
module_mipi_dsi_driver(samsung_amb655uv01_driver);

MODULE_DESCRIPTION("DRM driver for Samsung amb655uv01 panel");
MODULE_LICENSE("GPL");
